// SPDX-License-Identifier: Apache-2.0
// Android basic_flex_layout handler implementation.
//
// v2 strategy: host children in a plain android.widget.FrameLayout and drive
// the neutral, platform-independent `mpapp::flex_arrange` solver to compute
// each child's pixel rectangle. FrameLayout is a bare absolute-positioning
// container — every child is placed by its FrameLayout.LayoutParams
// {width, height, leftMargin, topMargin}, which is exactly the rect the
// solver produces. This replaces the v1 LinearLayout + weight approximation
// and makes wrap, align_content, shrink, basis, align_self and the *_reverse
// directions all honoured (the solver implements full CSS-flexbox math).
//
// Live reflow: the FrameLayout's allocated size is only known after Android
// lays it out, so the solve must re-run whenever that size changes. We attach
// an android.view.View.OnLayoutChangeListener (the io.mpapp.MppFlexLayoutListener
// Java shim) that calls back into native (nativeOnFlexLayoutChanged) with the
// new (w, h); the trampoline routes into relayout(). This mirrors the existing
// shape_view layout-listener pattern (MppShapeViewLayoutListener) and the
// MppActionRouter (ownerPtr, kind) trampoline convention.
//
// Java caveat: this handler relies on io.mpapp.MppFlexLayoutListener, a new
// one-method View.OnLayoutChangeListener placed in the android example's java
// tree. It must be compiled by the gradle build alongside the existing
// io.mpapp.Mpp* listener shims.

#include "mpapp/handlers/android/flex_layout_handler.hpp"

#if defined(__ANDROID__)

#include <vector>

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/layout/flex_arrange.hpp"

namespace mpapp::internal {

namespace {

// android.view.View.MeasureSpec.UNSPECIFIED == 0 (the mode bits live in the
// high 2 bits; mode 0 with a 0 size is an UNSPECIFIED spec). Passing this to
// View.measure asks the child for its natural content size.
constexpr int MEASURE_SPEC_UNSPECIFIED = 0;

// Construct an android.widget.FrameLayout(Context). Returns a global ref or
// nullptr on failure. Mirrors grid_layout_handler::make_grid_layout.
jobject make_frame_layout(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/FrameLayout");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck() || local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(cls);
        return nullptr;
    }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

// Add a child View to a ViewGroup with no explicit LayoutParams. The solver
// re-assigns LayoutParams in relayout(); this just inserts the child.
void add_view_simple(JNIEnv* env, jobject parent, jobject child) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "addView", "(Landroid/view/View;)V");
    if (m != nullptr) {
        env->CallVoidMethod(parent, m, child);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

// View.getWidth() / View.getHeight() — the allocated pixel size. Returns 0
// before Android has laid the view out.
int view_get_int(JNIEnv* env, jobject v, const char* method) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return 0; }
    jmethodID m = env->GetMethodID(cls, method, "()I");
    int result = 0;
    if (m != nullptr) {
        result = static_cast<int>(env->CallIntMethod(v, m));
        if (env->ExceptionCheck()) { env->ExceptionClear(); result = 0; }
    }
    env->DeleteLocalRef(cls);
    return result;
}

// Measure a child with UNSPECIFIED specs and read its measured content size.
// Used to fill flex_item_input.measured_main / measured_cross.
void view_measure_unspecified(JNIEnv* env, jobject v, int& out_w, int& out_h) {
    out_w = 0;
    out_h = 0;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID measure = env->GetMethodID(cls, "measure", "(II)V");
    jmethodID get_w   = env->GetMethodID(cls, "getMeasuredWidth", "()I");
    jmethodID get_h   = env->GetMethodID(cls, "getMeasuredHeight", "()I");
    if (measure != nullptr && get_w != nullptr && get_h != nullptr) {
        env->CallVoidMethod(v, measure,
                            MEASURE_SPEC_UNSPECIFIED, MEASURE_SPEC_UNSPECIFIED);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        } else {
            out_w = static_cast<int>(env->CallIntMethod(v, get_w));
            out_h = static_cast<int>(env->CallIntMethod(v, get_h));
            if (env->ExceptionCheck()) { env->ExceptionClear(); out_w = 0; out_h = 0; }
        }
    }
    env->DeleteLocalRef(cls);
}

// Build a FrameLayout.LayoutParams(width, height), set leftMargin/topMargin
// fields, and apply it via child.setLayoutParams(lp). Returns true on success.
bool apply_child_rect(JNIEnv* env, jobject child,
                      int width, int height, int left, int top) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass lp_cls = env->FindClass("android/widget/FrameLayout$LayoutParams");
    if (lp_cls == nullptr) { env->ExceptionClear(); return false; }
    jmethodID ctor = env->GetMethodID(lp_cls, "<init>", "(II)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(lp_cls); return false; }
    jobject lp = env->NewObject(lp_cls, ctor, width, height);
    if (env->ExceptionCheck() || lp == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(lp_cls);
        return false;
    }

    // leftMargin / topMargin are public int fields on
    // android.view.ViewGroup.MarginLayoutParams (the base of
    // FrameLayout.LayoutParams).
    jclass margin_cls = env->FindClass("android/view/ViewGroup$MarginLayoutParams");
    if (margin_cls != nullptr) {
        jfieldID left_fid = env->GetFieldID(margin_cls, "leftMargin", "I");
        jfieldID top_fid  = env->GetFieldID(margin_cls, "topMargin", "I");
        if (left_fid != nullptr) env->SetIntField(lp, left_fid, left);
        if (top_fid  != nullptr) env->SetIntField(lp, top_fid, top);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(margin_cls);
    }
    env->DeleteLocalRef(lp_cls);

    jclass view_cls = env->FindClass("android/view/View");
    bool ok = false;
    if (view_cls != nullptr) {
        jmethodID set_m = env->GetMethodID(
            view_cls, "setLayoutParams", "(Landroid/view/ViewGroup$LayoutParams;)V");
        if (set_m != nullptr) {
            env->CallVoidMethod(child, set_m, lp);
            if (env->ExceptionCheck()) { env->ExceptionClear(); } else { ok = true; }
        }
        env->DeleteLocalRef(view_cls);
    }
    env->DeleteLocalRef(lp);
    return ok;
}

void view_request_layout(JNIEnv* env, jobject v) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "requestLayout", "()V");
    if (m != nullptr) {
        env->CallVoidMethod(v, m);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

// Install io.mpapp.MppFlexLayoutListener on the FrameLayout. owner_ptr is the
// C++ handler `this`; the Java listener calls nativeOnFlexLayoutChanged with
// the new (w, h) whenever Android assigns the FrameLayout a new size, so the
// flex solve re-runs against real pixels. Mirrors shape_view's
// install_layout_listener.
void install_layout_listener(JNIEnv* env, jobject frame, jlong owner_ptr) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass listener_cls = env->FindClass("io/mpapp/MppFlexLayoutListener");
    if (listener_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID ctor = env->GetMethodID(listener_cls, "<init>", "(J)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(listener_cls); return; }
    jobject listener = env->NewObject(listener_cls, ctor, owner_ptr);
    if (env->ExceptionCheck() || listener == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(listener_cls);
        return;
    }
    env->DeleteLocalRef(listener_cls);

    jclass view_cls = env->FindClass("android/view/View");
    if (view_cls != nullptr) {
        jmethodID add_m = env->GetMethodID(
            view_cls,
            "addOnLayoutChangeListener",
            "(Landroid/view/View$OnLayoutChangeListener;)V");
        if (add_m != nullptr) {
            env->CallVoidMethod(frame, add_m, listener);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(view_cls);
    }
    env->DeleteLocalRef(listener);
}

} // namespace

flex_layout_handler<platform::android>::flex_layout_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_frame_layout(env, detail::get_activity());
    if (native_ != nullptr) {
        install_layout_listener(env, native_, reinterpret_cast<jlong>(this));
    }
}

flex_layout_handler<platform::android>::~flex_layout_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

// Re-run the flex solve against the FrameLayout's current measured size and
// push each child's computed rect into FrameLayout.LayoutParams. Called from
// add_child (best-effort, may be a no-op before first layout) and from the
// MppFlexLayoutListener trampoline (with real pixels).
void flex_layout_handler<platform::android>::relayout() {
    if (native_ == nullptr || bound_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    const int container_w = view_get_int(env, native_, "getWidth");
    const int container_h = view_get_int(env, native_, "getHeight");
    if (container_w <= 0 || container_h <= 0) {
        // Android hasn't laid the FrameLayout out yet; the
        // MppFlexLayoutListener callback will re-drive relayout() once it has.
        return;
    }

    basic_flex_layout& f = *bound_;
    const std::size_t n = f.child_count();
    if (n == 0) return;

    // Resolve each child's native View + measured content size + flex props.
    // child_objs is parallel to the flex_item_input vector and to the
    // flex_arrange result, so result index i drives child_objs[i].
    std::vector<jobject>          child_objs;
    std::vector<flex_item_input>  items;
    child_objs.reserve(n);
    items.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        view* child = f.child_at(i);
        if (child == nullptr) continue;
        jobject child_obj = detail::android_dispatch::dispatch(child);
        if (child_obj == nullptr) continue;

        int mw = 0;
        int mh = 0;
        view_measure_unspecified(env, child_obj, mw, mh);

        const auto props = f.get_child_props(*child);
        const bool horizontal =
            ::mpapp::internal::flex::is_row(f.direction.get());

        flex_item_input it;
        it.basis      = props.basis;
        it.grow       = props.grow;
        it.shrink     = props.shrink;
        it.align_self = props.align_self;
        it.order      = props.order;
        // measured_main/cross are axis-relative: along the main axis use the
        // measured extent in that axis, the cross extent for the cross axis.
        it.measured_main  = horizontal ? static_cast<double>(mw)
                                       : static_cast<double>(mh);
        it.measured_cross = horizontal ? static_cast<double>(mh)
                                       : static_cast<double>(mw);

        child_objs.push_back(child_obj);
        items.push_back(it);
    }

    if (items.empty()) return;

    flex_container_input container;
    container.width           = static_cast<double>(container_w);
    container.height          = static_cast<double>(container_h);
    container.direction       = f.direction.get();
    container.wrap            = f.wrap.get();
    container.justify_content = f.justify_content.get();
    container.align_items     = f.align_items.get();
    container.align_content   = f.align_content.get();

    const std::vector<flex_rect> rects = flex_arrange(container, items);

    for (std::size_t i = 0; i < rects.size() && i < child_objs.size(); ++i) {
        const flex_rect& r = rects[i];
        apply_child_rect(env, child_objs[i],
                         static_cast<int>(r.width),
                         static_cast<int>(r.height),
                         static_cast<int>(r.x),
                         static_cast<int>(r.y));
        env->DeleteLocalRef(child_objs[i]);
    }

    view_request_layout(env, native_);
}

void flex_layout_handler<platform::android>::on_layout_changed(int /*w*/, int /*h*/) {
    // The new size is read directly from getWidth/getHeight inside relayout();
    // the (w, h) the listener passes is only a change trigger.
    relayout();
}

void flex_layout_handler<platform::android>::apply_direction(flex_direction /*d*/) {
    relayout();
}

void flex_layout_handler<platform::android>::apply_wrap(flex_wrap /*w*/) {
    relayout();
}

void flex_layout_handler<platform::android>::apply_justify_content(flex_justify /*j*/) {
    relayout();
}

void flex_layout_handler<platform::android>::apply_align_items(flex_align_items /*a*/) {
    relayout();
}

void flex_layout_handler<platform::android>::apply_align_content(flex_align_content /*a*/) {
    relayout();
}

void flex_layout_handler<platform::android>::apply_position(flex_position /*p*/) {
    // flex_position::absolute (taking a child out of flow) is not yet modelled
    // by the neutral solver, which arranges every item in-flow. Treated as a
    // re-solve trigger; absolute children remain in-flow until the solver
    // grows an absolute-positioning path.
    relayout();
}

void flex_layout_handler<platform::android>::map_direction(basic_flex_layout& f) {
    bound_ = &f;
    apply_direction(f.direction.get());
    f.direction.changed.subscribe(direction_slot_, direction_cb_);
}

void flex_layout_handler<platform::android>::map_wrap(basic_flex_layout& f) {
    bound_ = &f;
    apply_wrap(f.wrap.get());
    f.wrap.changed.subscribe(wrap_slot_, wrap_cb_);
}

void flex_layout_handler<platform::android>::map_justify_content(basic_flex_layout& f) {
    bound_ = &f;
    apply_justify_content(f.justify_content.get());
    f.justify_content.changed.subscribe(justify_slot_, justify_cb_);
}

void flex_layout_handler<platform::android>::map_align_items(basic_flex_layout& f) {
    bound_ = &f;
    apply_align_items(f.align_items.get());
    f.align_items.changed.subscribe(align_items_slot_, align_items_cb_);
}

void flex_layout_handler<platform::android>::map_align_content(basic_flex_layout& f) {
    bound_ = &f;
    apply_align_content(f.align_content.get());
    f.align_content.changed.subscribe(align_content_slot_, align_content_cb_);
}

void flex_layout_handler<platform::android>::map_position(basic_flex_layout& f) {
    bound_ = &f;
    apply_position(f.position.get());
    f.position.changed.subscribe(position_slot_, position_cb_);
}

void flex_layout_handler<platform::android>::add_child(basic_flex_layout& f, view& child) {
    if (native_ == nullptr) return;
    bound_ = &f;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    // ADR-0013: registry dispatch only — each widget self-registers.
    jobject child_obj = detail::android_dispatch::dispatch(&child);
    if (child_obj == nullptr) return;

    // Insert into the FrameLayout; relayout() computes the actual rect. The
    // initial solve runs here (best-effort): if Android hasn't yet sized the
    // FrameLayout, getWidth/getHeight return 0 and relayout() bails — the
    // MppFlexLayoutListener callback then re-drives it once the size is known.
    add_view_simple(env, native_, child_obj);
    env->DeleteLocalRef(child_obj);

    relayout();
}

} // namespace mpapp::internal

// JNI trampoline for MppFlexLayoutListener.onLayoutChange. The Java listener
// supplies owner_ptr (the C++ handler `this`) plus the new (w, h); we
// reinterpret and route into on_layout_changed, which re-runs the solve.
// Mirrors Java_io_mpapp_MppShapeViewLayoutListener_nativeOnLayoutChanged.
extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppFlexLayoutListener_nativeOnFlexLayoutChanged(
    JNIEnv* /*env*/,
    jclass  /*cls*/,
    jlong   owner_ptr,
    jint    w,
    jint    h) {
    auto* self = reinterpret_cast<mpapp::internal::flex_layout_handler<mpapp::platform::android>*>(owner_ptr);
    if (self != nullptr) self->on_layout_changed(static_cast<int>(w), static_cast<int>(h));
}

// ---------- Self-registration with the per-platform dispatch registry ----
namespace {

jobject dispatch_flex_layout(::mpapp::view* v) {
    if (auto* f = dynamic_cast<::mpapp::internal::basic_flex_layout*>(v);
        f && f->has_handler()) {
        return f->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_flex_layout); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
