// SPDX-License-Identifier: Apache-2.0
// Android shell handler implementation.

#include "mpapp/handlers/android/shell_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp {

namespace {

constexpr int LINEAR_LAYOUT_VERTICAL   = 1;
constexpr int LINEAR_LAYOUT_HORIZONTAL = 0;
constexpr int VIEW_VISIBLE = 0;
constexpr int VIEW_GONE    = 8;

jobject make_object(JNIEnv* env, const char* cls_name, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass(cls_name);
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

void ll_set_orientation(JNIEnv* env, jobject ll, int orientation) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/LinearLayout");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setOrientation", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(ll, m, static_cast<jint>(orientation));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void view_set_text(JNIEnv* env, jobject view_obj, const char* text) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setText", "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring s = env->NewStringUTF(text);
        env->CallVoidMethod(view_obj, m, s);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(s);
    }
    env->DeleteLocalRef(cls);
}

void view_set_visibility(JNIEnv* env, jobject view_obj, int v) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setVisibility", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(view_obj, m, static_cast<jint>(v));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void vg_add(JNIEnv* env, jobject parent, jobject child) {
    if (child == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID add = env->GetMethodID(cls, "addView", "(Landroid/view/View;)V");
    if (add != nullptr) {
        env->CallVoidMethod(parent, add, child);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void vg_remove_all(JNIEnv* env, jobject parent) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "removeAllViews", "()V");
    if (m != nullptr) {
        env->CallVoidMethod(parent, m);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

shell_handler<platform::android>::shell_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject ctx = detail::get_activity();

    native_       = make_object(env, "android/widget/LinearLayout", ctx);
    flyout_host_  = make_object(env, "android/widget/FrameLayout",  ctx);
    main_host_    = make_object(env, "android/widget/LinearLayout", ctx);
    tab_strip_    = make_object(env, "android/widget/LinearLayout", ctx);
    content_host_ = make_object(env, "android/widget/FrameLayout",  ctx);

    if (native_    != nullptr) ll_set_orientation(env, native_,    LINEAR_LAYOUT_HORIZONTAL);
    if (main_host_ != nullptr) ll_set_orientation(env, main_host_, LINEAR_LAYOUT_VERTICAL);
    if (tab_strip_ != nullptr) ll_set_orientation(env, tab_strip_, LINEAR_LAYOUT_HORIZONTAL);

    if (flyout_host_ != nullptr) view_set_visibility(env, flyout_host_, VIEW_GONE);

    if (main_host_ != nullptr) {
        vg_add(env, main_host_, tab_strip_);
        vg_add(env, main_host_, content_host_);
    }
    if (native_ != nullptr) {
        vg_add(env, native_, flyout_host_);
        vg_add(env, native_, main_host_);
    }
}

shell_handler<platform::android>::~shell_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (content_host_ != nullptr) { env->DeleteGlobalRef(content_host_); content_host_ = nullptr; }
        if (tab_strip_    != nullptr) { env->DeleteGlobalRef(tab_strip_);    tab_strip_    = nullptr; }
        if (main_host_    != nullptr) { env->DeleteGlobalRef(main_host_);    main_host_    = nullptr; }
        if (flyout_host_  != nullptr) { env->DeleteGlobalRef(flyout_host_);  flyout_host_  = nullptr; }
        if (native_       != nullptr) { env->DeleteGlobalRef(native_);       native_       = nullptr; }
    }
}

void shell_handler<platform::android>::rebuild_tab_strip(const std::vector<std::string>& labels) {
    if (tab_strip_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    vg_remove_all(env, tab_strip_);
    jobject ctx = detail::get_activity();
    for (std::size_t i = 0; i < labels.size(); ++i) {
        jobject btn = make_object(env, "android/widget/Button", ctx);
        if (btn != nullptr) {
            view_set_text(env, btn, labels[i].c_str());
            vg_add(env, tab_strip_, btn);
            env->DeleteGlobalRef(btn);
        }
    }
    // OnClickListener wiring to set current_tab_index is deferred to
    // M-05 polish — same approach as the navigation_page Android handler.
}

void shell_handler<platform::android>::apply_selection(int /*idx*/) {
    // Visual highlight deferred.
}

void shell_handler<platform::android>::apply_is_flyout_open(bool v) {
    if (flyout_host_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    view_set_visibility(env, flyout_host_, v ? VIEW_VISIBLE : VIEW_GONE);
}

void shell_handler<platform::android>::apply_flyout_content(page* p) {
    if (flyout_host_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    vg_remove_all(env, flyout_host_);
    if (p != nullptr) {
        if (jobject n = detail::android_dispatch::dispatch(p); n != nullptr) {
            vg_add(env, flyout_host_, n);
        }
    }
}

void shell_handler<platform::android>::apply_current_content(page* p) {
    if (content_host_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    vg_remove_all(env, content_host_);
    if (p != nullptr) {
        if (jobject n = detail::android_dispatch::dispatch(p); n != nullptr) {
            vg_add(env, content_host_, n);
        }
    }
}

void shell_handler<platform::android>::map_tabs(shell& s) {
    bound_ = &s;
    rebuild_tab_strip(s.tabs.get());
    s.tabs.changed.subscribe(tabs_slot_, tabs_cb_);
}

void shell_handler<platform::android>::map_current_tab_index(shell& s) {
    apply_selection(s.current_tab_index.get());
    s.current_tab_index.changed.subscribe(sel_slot_, sel_cb_);
}

void shell_handler<platform::android>::map_is_flyout_open(shell& s) {
    apply_is_flyout_open(s.is_flyout_open.get());
    s.is_flyout_open.changed.subscribe(flyout_open_slot_, flyout_open_cb_);
}

void shell_handler<platform::android>::map_flyout_content(shell& s) {
    apply_flyout_content(s.flyout_content.get());
    s.flyout_content.changed.subscribe(flyout_content_slot_, flyout_content_cb_);
}

void shell_handler<platform::android>::map_current_content(shell& s) {
    apply_current_content(s.current_content.get());
    s.current_content.changed.subscribe(content_slot_, content_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_shell(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::shell*>(v); s && s->has_shell_handler()) {
        return s->shell_handler_ref().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_shell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
