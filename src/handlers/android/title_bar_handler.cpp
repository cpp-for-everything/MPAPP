// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android title_bar handler implementation.

#include "mpapp/handlers/android/title_bar_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp {

namespace {

jobject make_toolbar(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/Toolbar");
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

void toolbar_set_text(JNIEnv* env, jobject toolbar,
                      const char* setter, const std::string& v) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/Toolbar");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, setter, "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring jstr = env->NewStringUTF(v.c_str());
        env->CallVoidMethod(toolbar, m, jstr);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(cls);
}

} // namespace

title_bar_handler<platform::android>::title_bar_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env != nullptr) {
        native_ = make_toolbar(env, detail::get_activity());
    }
}

title_bar_handler<platform::android>::~title_bar_handler() {
    if (native_ != nullptr) {
        if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
            env->DeleteGlobalRef(native_);
        }
        native_ = nullptr;
    }
}

void title_bar_handler<platform::android>::apply_title(const std::string& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    toolbar_set_text(env, native_, "setTitle", v);
}

void title_bar_handler<platform::android>::apply_subtitle(const std::string& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    toolbar_set_text(env, native_, "setSubtitle", v);
}

void title_bar_handler<platform::android>::map_title(title_bar& t) {
    apply_title(t.title.get());
    t.title.changed.subscribe(title_slot_, title_cb_);
}
void title_bar_handler<platform::android>::map_subtitle(title_bar& t) {
    apply_subtitle(t.subtitle.get());
    t.subtitle.changed.subscribe(subtitle_slot_, subtitle_cb_);
}

} // namespace mpapp

// --- ADR-0013 self-registration --------------------------------------------

namespace {

jobject dispatch_title_bar(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::title_bar*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_title_bar);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
