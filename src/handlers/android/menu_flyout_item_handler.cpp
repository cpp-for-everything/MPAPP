// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_menu_flyout_item handler implementation.

#include "mpapp/handlers/android/menu_flyout_item_handler.hpp"

#if defined(__ANDROID__)

#include <string>

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

#include "mpapp/internal/basic_menu_flyout_item.hpp"

namespace mpapp::internal {

namespace {

jobject make_button(JNIEnv* env, jobject context) {
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
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

void button_set_text(JNIEnv* env, jobject btn, const std::string& text) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/Button");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setText", "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring jstr = env->NewStringUTF(text.c_str());
        env->CallVoidMethod(btn, m, jstr);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(cls);
}

void view_set_enabled(JNIEnv* env, jobject v, bool enabled) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setEnabled", "(Z)V");
    if (m != nullptr) {
        env->CallVoidMethod(v, m, enabled ? JNI_TRUE : JNI_FALSE);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

menu_flyout_item_handler<platform::android>::menu_flyout_item_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_button(env, detail::get_activity());
}

menu_flyout_item_handler<platform::android>::~menu_flyout_item_handler() {
    if (native_ != nullptr) {
        if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
            env->DeleteGlobalRef(native_);
        }
        native_ = nullptr;
    }
}

void menu_flyout_item_handler<platform::android>::apply_text(const std::string& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    button_set_text(env, native_, v);
}

void menu_flyout_item_handler<platform::android>::apply_is_enabled(bool v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    view_set_enabled(env, native_, v);
}

void menu_flyout_item_handler<platform::android>::map_text(basic_menu_flyout_item& i) {
    owner_ = &i;
    apply_text(i.text.get());
    i.text.changed.subscribe(text_slot_, text_cb_);
    // Click wiring deferred to M-05: the existing `MppClickRouter`
    // (used by button_handler) is hard-coded to dispatch into
    // `mpapp::basic_button*`. A `MppMenuItemClickRouter` lands alongside the
    // M-05 context-flyout work — adding new Java classes is outside
    // the M-04b worker scope.
    listener_ = false;
}

void menu_flyout_item_handler<platform::android>::map_is_enabled(basic_menu_flyout_item& i) {
    apply_is_enabled(i.is_enabled.get());
    i.is_enabled.changed.subscribe(is_enabled_slot_, is_enabled_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --

namespace {

jobject dispatch_menu_flyout_item(::mpapp::view* v) {
    if (auto* i = dynamic_cast<::mpapp::internal::basic_menu_flyout_item*>(v); i && i->has_handler()) {
        return i->handler().native();
    }
    return nullptr;
}

struct registrar_mfi {
    registrar_mfi() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_menu_flyout_item); }
};

[[maybe_unused]] registrar_mfi _reg;

} // namespace

#endif // __ANDROID__
