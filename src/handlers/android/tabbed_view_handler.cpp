// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_tabbed_view handler implementation.
//
// See the corresponding header for layout shape and why we deliberately
// avoid TabLayout/ViewPager2 (androidx) and TabHost (heavyweight setup).
// The native handle is a vertical LinearLayout whose first child is a
// horizontal tab strip (one Button per title) and whose second child is
// a FrameLayout (the basic_page host) whose children's visibility tracks
// `selected_index`.

#include "mpapp/handlers/android/tabbed_view_handler.hpp"

#if defined(__ANDROID__)

#include <string>
#include <vector>

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

constexpr int LINEAR_LAYOUT_HORIZONTAL = 0;
constexpr int LINEAR_LAYOUT_VERTICAL   = 1;
constexpr int VIEW_VISIBLE             = 0;
constexpr int VIEW_GONE                = 8;

// Set the LinearLayout orientation, swallowing JNI exceptions
// uniformly with the rest of the Android handler conventions.
void linear_layout_set_orientation(JNIEnv* env, jobject ll, jint orient) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/LinearLayout");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setOrientation", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(ll, m, orient);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

// Create a `LinearLayout` with the given orientation, returning a global
// ref the caller owns. Returns nullptr on any JNI failure.
jobject make_linear_layout(JNIEnv* env, jobject context, jint orient) {
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
    linear_layout_set_orientation(env, local, orient);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

// Create a bare `FrameLayout`. Returns a global ref the caller owns.
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

// Create a `Button` with the given text; returns a local ref the
// caller should attach to a parent and then DeleteLocalRef.
jobject make_button(JNIEnv* env, jobject context, const std::string& text) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/Button");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck() || local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(cls);
        return nullptr;
    }
    // Button extends TextView — use TextView.setText for the basic_label.
    jclass tv_cls = env->FindClass("android/widget/TextView");
    if (tv_cls != nullptr) {
        jmethodID set_text = env->GetMethodID(
            tv_cls, "setText", "(Ljava/lang/CharSequence;)V");
        if (set_text != nullptr) {
            jstring js = env->NewStringUTF(text.c_str());
            env->CallVoidMethod(local, set_text, js);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(js);
        }
        env->DeleteLocalRef(tv_cls);
    }
    env->DeleteLocalRef(cls);
    return local;
}

void view_group_add(JNIEnv* env, jobject group, jobject child) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "addView", "(Landroid/view/View;)V");
    if (m != nullptr) {
        env->CallVoidMethod(group, m, child);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void view_group_remove_all(JNIEnv* env, jobject group) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "removeAllViews", "()V");
    if (m != nullptr) {
        env->CallVoidMethod(group, m);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

jint view_group_get_child_count(JNIEnv* env, jobject group) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return 0; }
    jmethodID m = env->GetMethodID(cls, "getChildCount", "()I");
    jint count = 0;
    if (m != nullptr) {
        count = env->CallIntMethod(group, m);
        if (env->ExceptionCheck()) { env->ExceptionClear(); count = 0; }
    }
    env->DeleteLocalRef(cls);
    return count;
}

jobject view_group_get_child_at(JNIEnv* env, jobject group, jint index) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID m = env->GetMethodID(cls, "getChildAt", "(I)Landroid/view/View;");
    jobject child = nullptr;
    if (m != nullptr) {
        child = env->CallObjectMethod(group, m, index);
        if (env->ExceptionCheck()) { env->ExceptionClear(); child = nullptr; }
    }
    env->DeleteLocalRef(cls);
    return child;  // local ref
}

void view_set_visibility(JNIEnv* env, jobject view_obj, jint visibility) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID set_vis = env->GetMethodID(cls, "setVisibility", "(I)V");
    if (set_vis != nullptr) {
        env->CallVoidMethod(view_obj, set_vis, visibility);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

tabbed_view_handler<platform::android>::tabbed_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject context = detail::get_activity();

    // Outer host: vertical LinearLayout (tab strip on top, basic_page area
    // below). We deliberately avoid `android.widget.TabHost` here —
    // setting up TabHost requires either inflating a layout resource
    // (we have none in `android_hello`) or programmatically resolving
    // android.R.id.tabs / android.R.id.tabcontent and wiring a
    // TabContentFactory through JNI, both of which are heavyweight for
    // a primitive whose only required behaviour is "show one of N
    // labelled pages at a time".
    native_ = make_linear_layout(env, context, LINEAR_LAYOUT_VERTICAL);
    if (native_ == nullptr) return;

    // Tab strip: horizontal LinearLayout that holds one Button per
    // title. The strip is attached but starts empty; apply_tab_titles
    // populates it. We don't keep a separate global ref to it — it's
    // findable as child 0 of the outer host.
    jobject strip = make_linear_layout(env, context, LINEAR_LAYOUT_HORIZONTAL);
    if (strip != nullptr) {
        view_group_add(env, native_, strip);
        env->DeleteGlobalRef(strip);
    }

    // Content frame: FrameLayout that holds one placeholder child per
    // tab. We keep a global ref so apply_selected_index can walk its
    // children without re-resolving by index every time.
    content_frame_ = make_frame_layout(env, context);
    if (content_frame_ != nullptr) {
        view_group_add(env, native_, content_frame_);
    }
}

tabbed_view_handler<platform::android>::~tabbed_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (content_frame_ != nullptr) { env->DeleteGlobalRef(content_frame_); content_frame_ = nullptr; }
    if (native_        != nullptr) { env->DeleteGlobalRef(native_);        native_        = nullptr; }
}

void tabbed_view_handler<platform::android>::apply_tab_titles(const std::vector<std::string>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (suppress_echo_) return;
    suppress_echo_ = true;

    jobject context = detail::get_activity();

    // The tab strip is the first child of `native_`. Clear it and
    // rebuild from `v`.
    jobject strip = view_group_get_child_at(env, native_, 0);
    if (strip != nullptr) {
        view_group_remove_all(env, strip);
        for (const auto& title : v) {
            jobject btn = make_button(env, context, title);
            if (btn != nullptr) {
                view_group_add(env, strip, btn);
                env->DeleteLocalRef(btn);
            }
        }
        env->DeleteLocalRef(strip);
    }

    // Rebuild the content frame's placeholders — one per tab.
    if (content_frame_ != nullptr) {
        view_group_remove_all(env, content_frame_);
        for (size_t i = 0; i < v.size(); ++i) {
            jobject basic_page = make_frame_layout(env, context);
            if (basic_page != nullptr) {
                // The first placeholder stays visible by default; the
                // rest are GONE until apply_selected_index reveals one
                // of them. This matches the "single visible basic_page at
                // a time" cross-platform contract.
                view_set_visibility(env, basic_page, (i == 0) ? VIEW_VISIBLE : VIEW_GONE);
                view_group_add(env, content_frame_, basic_page);
                env->DeleteGlobalRef(basic_page);
            }
        }
    }

    suppress_echo_ = false;
}

void tabbed_view_handler<platform::android>::apply_selected_index(int v) {
    if (native_ == nullptr || content_frame_ == nullptr) return;
    if (v < 0) return;  // -1 ⇒ leave the current basic_page visible
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();

    const jint count = view_group_get_child_count(env, content_frame_);
    if (v >= count) return;

    // Set GONE on every basic_page, then VISIBLE on the chosen one. Each
    // child ref is local — we release it after toggling.
    for (jint i = 0; i < count; ++i) {
        jobject child = view_group_get_child_at(env, content_frame_, i);
        if (child == nullptr) continue;
        view_set_visibility(env, child, (i == v) ? VIEW_VISIBLE : VIEW_GONE);
        env->DeleteLocalRef(child);
    }
}

void tabbed_view_handler<platform::android>::map_tab_titles(basic_tabbed_view& t) {
    apply_tab_titles(t.tab_titles.get());
    t.tab_titles.changed.subscribe(tab_titles_slot_, tab_titles_cb_);
}

void tabbed_view_handler<platform::android>::map_selected_index(basic_tabbed_view& t) {
    apply_selected_index(t.selected_index.get());
    t.selected_index.changed.subscribe(selected_index_slot_, selected_index_cb_);
}

// ----- ADR-0013 self-registration --------------------------------------

namespace {

jobject dispatch_tabbed_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_tabbed_view*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_tabbed_view);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

} // namespace mpapp::internal
#endif // __ANDROID__
