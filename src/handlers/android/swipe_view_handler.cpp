// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android swipe_view handler implementation.

#include "mpapp/handlers/android/swipe_view_handler.hpp"

#if defined(__ANDROID__)

#include <vector>

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

swipe_view_handler<platform::android>::swipe_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    native_ = make_frame_layout(env, detail::get_activity());
}

swipe_view_handler<platform::android>::~swipe_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (current_child_ != nullptr) { env->DeleteGlobalRef(current_child_); current_child_ = nullptr; }
    if (native_        != nullptr) { env->DeleteGlobalRef(native_);        native_        = nullptr; }
}

void swipe_view_handler<platform::android>::apply_content(view* v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    if (current_child_ != nullptr) {
        view_group_remove(env, native_, current_child_);
        env->DeleteGlobalRef(current_child_);
        current_child_ = nullptr;
    }

    // ADR-0013: query the registry.
    jobject child = (v != nullptr) ? detail::android_dispatch::dispatch(v) : nullptr;
    if (child == nullptr) return;

    current_child_ = env->NewGlobalRef(child);
    view_group_add(env, native_, current_child_);
}

void swipe_view_handler<platform::android>::apply_left_items(const std::vector<view*>& items) {
    // Gesture-revealed pane composition is deferred. We still walk the
    // registry per entry so any child's native handle gets materialised
    // (mirrors the Linux degradation path).
    for (view* v : items) {
        if (v != nullptr) (void)detail::android_dispatch::dispatch(v);
    }
}

void swipe_view_handler<platform::android>::apply_right_items(const std::vector<view*>& items) {
    for (view* v : items) {
        if (v != nullptr) (void)detail::android_dispatch::dispatch(v);
    }
}

void swipe_view_handler<platform::android>::map_content(swipe_view& sv) {
    apply_content(sv.content.get());
    sv.content.changed.subscribe(content_slot_, content_cb_);
}

void swipe_view_handler<platform::android>::map_left_items(swipe_view& sv) {
    apply_left_items(sv.left_items.get());
    sv.left_items.changed.subscribe(left_slot_, left_cb_);
}

void swipe_view_handler<platform::android>::map_right_items(swipe_view& sv) {
    apply_right_items(sv.right_items.get());
    sv.right_items.changed.subscribe(right_slot_, right_cb_);
}

} // namespace mpapp

// ----- ADR-0013 self-registration --------------------------------------

namespace {

jobject dispatch_swipe_view(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::swipe_view*>(v); s && s->has_handler()) {
        return s->handler().native();
    }
    return nullptr;
}

struct swipe_view_registrar {
    swipe_view_registrar() {
        ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_swipe_view);
    }
};

[[maybe_unused]] swipe_view_registrar _swipe_view_reg;

} // namespace

#endif // __ANDROID__
