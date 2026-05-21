// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android menu_flyout_separator handler implementation.

#include "mpapp/handlers/android/menu_flyout_separator_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

#include "mpapp/menu_flyout_separator.hpp"

namespace mpapp {

namespace {

jobject make_separator_view(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck() || local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(cls);
        return nullptr;
    }
    // Set minimum height to 1px so the parent layout reserves vertical
    // space for the divider. setMinimumHeight(int) on android.view.View.
    jmethodID set_min_h = env->GetMethodID(cls, "setMinimumHeight", "(I)V");
    if (set_min_h != nullptr) {
        env->CallVoidMethod(local, set_min_h, 1);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

} // namespace

menu_flyout_separator_handler<platform::android>::menu_flyout_separator_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_separator_view(env, detail::get_activity());
}

menu_flyout_separator_handler<platform::android>::~menu_flyout_separator_handler() {
    if (native_ != nullptr) {
        if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
            env->DeleteGlobalRef(native_);
        }
        native_ = nullptr;
    }
}

} // namespace mpapp

// ---------- Self-registration with the per-platform dispatch registry --

namespace {

jobject dispatch_menu_flyout_separator(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::menu_flyout_separator*>(v); s && s->has_handler()) {
        return s->handler().native();
    }
    return nullptr;
}

struct registrar_mfs {
    registrar_mfs() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_menu_flyout_separator); }
};

[[maybe_unused]] registrar_mfs _reg;

} // namespace

#endif // __ANDROID__
