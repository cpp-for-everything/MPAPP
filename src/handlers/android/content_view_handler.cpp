// SPDX-License-Identifier: Apache-2.0
// Android content_view handler implementation.

#include "mpapp/handlers/android/content_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp {

namespace {

// ADR-0013: ask the per-platform dispatch registry. Each widget's .cpp
// self-registers a dispatcher; the registry tries each in order and
// returns the first non-null. Replaces the legacy dynamic_cast chain.
jobject child_jobject(view* v) {
    return ::mpapp::detail::android_dispatch::dispatch(v);
}

jobject make_frame_layout(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/FrameLayout");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck() || local == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

} // namespace

content_view_handler<platform::android>::content_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_frame_layout(env, detail::get_activity());
}

content_view_handler<platform::android>::~content_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void content_view_handler<platform::android>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass vg = env->FindClass("android/view/ViewGroup");
    if (vg == nullptr) { env->ExceptionClear(); return; }
    jmethodID clear_m = env->GetMethodID(vg, "removeAllViews", "()V");
    if (clear_m != nullptr) {
        env->CallVoidMethod(native_, clear_m);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    jobject child = v ? child_jobject(v.get()) : nullptr;
    if (child != nullptr) {
        jmethodID add_m = env->GetMethodID(vg, "addView", "(Landroid/view/View;)V");
        if (add_m != nullptr) {
            env->CallVoidMethod(native_, add_m, child);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
    }
    env->DeleteLocalRef(vg);
}

void content_view_handler<platform::android>::map_content(content_view& c) {
    apply_content(c.content.get());
    c.content.changed.subscribe(content_slot_, content_cb_);
}

void content_view_handler<platform::android>::bind_content(content_view& c, view& child) {
    c.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp

// ---------- Self-registration with the per-platform dispatch registry --
#include "mpapp/content_view.hpp"

namespace {

jobject dispatch_content_view(::mpapp::view* v) {
    if (auto* cv = dynamic_cast<::mpapp::content_view*>(v); cv && cv->has_handler()) {
        return cv->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_content_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
