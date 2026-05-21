// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android swipe_item_view handler implementation.

#include "mpapp/handlers/android/swipe_item_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp {

namespace {

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

void view_group_remove(JNIEnv* env, jobject group, jobject child) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "removeView", "(Landroid/view/View;)V");
    if (m != nullptr) {
        env->CallVoidMethod(group, m, child);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

swipe_item_view_handler<platform::android>::swipe_item_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    native_ = make_frame_layout(env, detail::get_activity());
}

swipe_item_view_handler<platform::android>::~swipe_item_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (current_child_ != nullptr) { env->DeleteGlobalRef(current_child_); current_child_ = nullptr; }
    if (native_        != nullptr) { env->DeleteGlobalRef(native_);        native_        = nullptr; }
}

void swipe_item_view_handler<platform::android>::apply_content(view* v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    if (current_child_ != nullptr) {
        view_group_remove(env, native_, current_child_);
        env->DeleteGlobalRef(current_child_);
        current_child_ = nullptr;
    }

    jobject child = (v != nullptr) ? detail::android_dispatch::dispatch(v) : nullptr;
    if (child == nullptr) return;

    current_child_ = env->NewGlobalRef(child);
    view_group_add(env, native_, current_child_);
}

void swipe_item_view_handler<platform::android>::map_content(swipe_item_view& iv) {
    apply_content(iv.content.get());
    iv.content.changed.subscribe(content_slot_, content_cb_);
}

} // namespace mpapp

// ----- ADR-0013 self-registration --------------------------------------

namespace {

jobject dispatch_swipe_item_view(::mpapp::view* v) {
    if (auto* iv = dynamic_cast<::mpapp::swipe_item_view*>(v); iv && iv->has_handler()) {
        return iv->handler().native();
    }
    return nullptr;
}

struct swipe_item_view_registrar {
    swipe_item_view_registrar() {
        ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_swipe_item_view);
    }
};

[[maybe_unused]] swipe_item_view_registrar _swipe_item_view_reg;

} // namespace

#endif // __ANDROID__
