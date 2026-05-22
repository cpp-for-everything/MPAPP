// SPDX-License-Identifier: Apache-2.0
// Android graphics_view handler implementation.

#include "mpapp/handlers/android/graphics_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp {

namespace {

jobject make_view(JNIEnv* env, jobject context) {
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
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

void view_set_min_size(JNIEnv* env, jobject v, const char* method, int px) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, method, "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(v, m, static_cast<jint>(px));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

graphics_view_handler<platform::android>::graphics_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_view(env, detail::get_activity());
}

graphics_view_handler<platform::android>::~graphics_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void graphics_view_handler<platform::android>::apply_width(int w) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    view_set_min_size(env, native_, "setMinimumWidth", w);
}

void graphics_view_handler<platform::android>::apply_height(int h) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    view_set_min_size(env, native_, "setMinimumHeight", h);
}

void graphics_view_handler<platform::android>::map_size(graphics_view& gv) {
    apply_width(gv.width.get());
    apply_height(gv.height.get());
    gv.width.changed.subscribe(w_slot_, w_cb_);
    gv.height.changed.subscribe(h_slot_, h_cb_);
}

void graphics_view_handler<platform::android>::map_draw_count(graphics_view& /*gv*/) {
    // No-op for v1; see ADR-0015 follow-up.
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_graphics_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::graphics_view*>(v); w && w->has_gv_handler()) {
        return w->gv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_graphics_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
