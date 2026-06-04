// SPDX-License-Identifier: Apache-2.0
// Android basic_flex_layout handler implementation.
//
// v1 strategy: map the flex container onto android.widget.LinearLayout —
// the same native widget the stack-layout handler drives. This gives a
// single-line (no-wrap) flex container that honours flex_direction,
// per-child `grow` (via LinearLayout.LayoutParams weight), and an
// approximation of justify_content / align_items via LinearLayout gravity.
//
// LinearLayout is a deliberate v1 shim, NOT a faithful flexbox:
//   - flex_wrap is ignored (LinearLayout is always single-line).
//   - align_content (multi-line cross-axis distribution) has no meaning
//     on a single line and is a no-op.
//   - shrink / basis / align_self / order per-child props are not honoured.
//   - flex_position::absolute is not supported.
//   - row_reverse / column_reverse fall back to their forward direction.
//
// FOLLOW-UP: a faithful flexbox should wrap com.google.android.flexbox
// .FlexboxLayout (the AndroidX Flexbox library) so that wrap, align_content,
// shrink, basis, align_self and order are all honoured. That is tracked as
// a separate task; this file intentionally confines itself to the platform
// widgets already available without an extra Gradle dependency.

#include "mpapp/handlers/android/flex_layout_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

// android.widget.LinearLayout orientation constants.
constexpr int LINEAR_LAYOUT_HORIZONTAL = 0;
constexpr int LINEAR_LAYOUT_VERTICAL   = 1;

// android.view.Gravity constants (the subset we map onto).
constexpr int GRAVITY_START           = 0x00800003;
constexpr int GRAVITY_END             = 0x00800005;
constexpr int GRAVITY_FILL_HORIZONTAL = 0x00000007;
constexpr int GRAVITY_TOP             = 0x00000030;
constexpr int GRAVITY_BOTTOM          = 0x00000050;
constexpr int GRAVITY_FILL_VERTICAL   = 0x00000070;
constexpr int GRAVITY_CENTER          = 0x00000011;

// LinearLayout.LayoutParams MATCH_PARENT / WRAP_CONTENT sentinels.
constexpr int LP_MATCH_PARENT = -1;
constexpr int LP_WRAP_CONTENT = -2;

// Construct an android.widget.LinearLayout(Context). Returns a global ref
// or nullptr on failure. Mirrors stack_layout_handler::make_linear_layout.
jobject make_linear_layout(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/LinearLayout");
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

void linear_layout_set_orientation(JNIEnv* env, jobject ll, int orient_native) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/LinearLayout");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setOrientation", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(ll, m, orient_native);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void linear_layout_set_gravity(JNIEnv* env, jobject ll, int gravity) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/LinearLayout");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setGravity", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(ll, m, gravity);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

bool is_horizontal(flex_direction d) noexcept {
    return d == flex_direction::row || d == flex_direction::row_reverse;
}

// justify_content distributes children along the MAIN axis. Because the
// declaration-only handler keeps no axis state (per the header, the only
// private members are the slot/callback machinery and `native_`), each
// apply_* call is self-contained: it sets BOTH the horizontal and vertical
// gravity components so the result reads correctly whether the container's
// flex_direction is row or column. space_* modes have no LinearLayout
// equivalent and fall back to start (a FlexboxLayout follow-up concern).
int justify_gravity(flex_justify j) noexcept {
    switch (j) {
        case flex_justify::center:
            return GRAVITY_CENTER;
        case flex_justify::end:
            return GRAVITY_END | GRAVITY_BOTTOM;
        case flex_justify::start:
        case flex_justify::space_between:
        case flex_justify::space_around:
        case flex_justify::space_evenly:
            break;
    }
    return GRAVITY_START | GRAVITY_TOP;
}

// align_items aligns children along the CROSS axis. As with justify_gravity,
// both axes are set so the mapping holds regardless of flex_direction.
int align_items_gravity(flex_align_items a) noexcept {
    switch (a) {
        case flex_align_items::center:
            return GRAVITY_CENTER;
        case flex_align_items::start:
            return GRAVITY_START | GRAVITY_TOP;
        case flex_align_items::end:
            return GRAVITY_END | GRAVITY_BOTTOM;
        case flex_align_items::stretch:
            break;
    }
    return GRAVITY_FILL_HORIZONTAL | GRAVITY_FILL_VERTICAL;
}

// Build a LinearLayout.LayoutParams(width, height, weight) for a child,
// where `weight` carries the flex `grow` factor. Returns a local ref or
// nullptr on failure.
jobject make_child_layout_params(JNIEnv* env, int width, int height, float weight) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass lp_cls = env->FindClass("android/widget/LinearLayout$LayoutParams");
    if (lp_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(lp_cls, "<init>", "(IIF)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(lp_cls); return nullptr; }
    jobject lp = env->NewObject(lp_cls, ctor, width, height, weight);
    env->DeleteLocalRef(lp_cls);
    if (env->ExceptionCheck() || lp == nullptr) {
        env->ExceptionClear();
        return nullptr;
    }
    return lp;
}

void add_view_with_params(JNIEnv* env, jobject parent, jobject child, jobject lp) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "addView",
        "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");
    if (m != nullptr) {
        env->CallVoidMethod(parent, m, child, lp);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

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

} // namespace

flex_layout_handler<platform::android>::flex_layout_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_linear_layout(env, detail::get_activity());
}

flex_layout_handler<platform::android>::~flex_layout_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void flex_layout_handler<platform::android>::apply_direction(flex_direction d) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    linear_layout_set_orientation(env, native_,
        is_horizontal(d) ? LINEAR_LAYOUT_HORIZONTAL : LINEAR_LAYOUT_VERTICAL);
}

void flex_layout_handler<platform::android>::apply_wrap(flex_wrap /*w*/) {
    // LinearLayout is always single-line; wrap has no LinearLayout
    // equivalent. Honouring it requires the FlexboxLayout follow-up.
}

void flex_layout_handler<platform::android>::apply_justify_content(flex_justify j) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    // v1 limitation: LinearLayout has a single container gravity, so justify
    // and align_items share it — whichever is applied last wins. A faithful
    // mapping (and space_* distribution) is a FlexboxLayout follow-up.
    linear_layout_set_gravity(env, native_, justify_gravity(j));
}

void flex_layout_handler<platform::android>::apply_align_items(flex_align_items a) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    // See apply_justify_content: container gravity is shared in v1.
    linear_layout_set_gravity(env, native_, align_items_gravity(a));
}

void flex_layout_handler<platform::android>::apply_align_content(flex_align_content /*a*/) {
    // align_content distributes multiple lines on the cross axis. A
    // single-line LinearLayout has no lines to distribute, so this is a
    // no-op until the FlexboxLayout follow-up.
}

void flex_layout_handler<platform::android>::apply_position(flex_position /*p*/) {
    // flex_position::absolute is not expressible on LinearLayout. v1 treats
    // every child as relative (in-flow). Deferred to the FlexboxLayout
    // follow-up.
}

void flex_layout_handler<platform::android>::map_direction(basic_flex_layout& f) {
    apply_direction(f.direction.get());
    f.direction.changed.subscribe(direction_slot_, direction_cb_);
}

void flex_layout_handler<platform::android>::map_wrap(basic_flex_layout& f) {
    apply_wrap(f.wrap.get());
    f.wrap.changed.subscribe(wrap_slot_, wrap_cb_);
}

void flex_layout_handler<platform::android>::map_justify_content(basic_flex_layout& f) {
    apply_justify_content(f.justify_content.get());
    f.justify_content.changed.subscribe(justify_slot_, justify_cb_);
}

void flex_layout_handler<platform::android>::map_align_items(basic_flex_layout& f) {
    apply_align_items(f.align_items.get());
    f.align_items.changed.subscribe(align_items_slot_, align_items_cb_);
}

void flex_layout_handler<platform::android>::map_align_content(basic_flex_layout& f) {
    apply_align_content(f.align_content.get());
    f.align_content.changed.subscribe(align_content_slot_, align_content_cb_);
}

void flex_layout_handler<platform::android>::map_position(basic_flex_layout& f) {
    apply_position(f.position.get());
    f.position.changed.subscribe(position_slot_, position_cb_);
}

void flex_layout_handler<platform::android>::add_child(basic_flex_layout& f, view& child) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    // ADR-0013: registry dispatch only — each widget self-registers.
    jobject child_obj = detail::android_dispatch::dispatch(&child);
    if (child_obj == nullptr) return;

    const auto props = f.get_child_props(child);

    // Map flex `grow` onto LinearLayout weight. A child with weight > 0
    // expands along the main axis; its main-axis dimension is then set to
    // 0 so the weight (not the measured size) governs the distribution —
    // the idiomatic LinearLayout weight pattern. Children with grow == 0
    // size to their content on the main axis.
    const bool  horizontal = is_horizontal(f.direction.get());
    const float weight     = static_cast<float>(props.grow);
    const bool  grows      = props.grow > 0.0;

    const int main_dim  = grows ? 0 : LP_WRAP_CONTENT;
    // Cross-axis stretch maps to MATCH_PARENT, otherwise WRAP_CONTENT.
    const int cross_dim = (f.align_items.get() == flex_align_items::stretch)
                              ? LP_MATCH_PARENT : LP_WRAP_CONTENT;

    const int width  = horizontal ? main_dim  : cross_dim;
    const int height = horizontal ? cross_dim : main_dim;

    if (jobject lp = make_child_layout_params(env, width, height, weight);
        lp != nullptr) {
        add_view_with_params(env, native_, child_obj, lp);
        env->DeleteLocalRef(lp);
    } else {
        // LayoutParams construction failed — still attach the child so it
        // is visible, just without weight-driven sizing.
        add_view_simple(env, native_, child_obj);
    }
}

} // namespace mpapp::internal

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
