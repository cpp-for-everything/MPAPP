// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android switch handler implementation.

#include "mpapp/handlers/android/switch_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp::internal {

namespace {

jobject make_switch(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/Switch");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    env->DeleteLocalRef(cls);
    if (local == nullptr) return nullptr;
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

void switch_set_checked(JNIEnv* env, jobject sw, bool checked) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/CompoundButton");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setChecked", "(Z)V");
    if (m != nullptr) {
        env->CallVoidMethod(sw, m, static_cast<jboolean>(checked ? JNI_TRUE : JNI_FALSE));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

jobject install_checked_change_listener(JNIEnv* env, jobject sw, jlong handler_ptr) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass listener_cls = env->FindClass("io/mpapp/MppCheckedChangeListener");
    if (listener_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(listener_cls, "<init>", "(JI)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(listener_cls); return nullptr; }
    // kind=1 → switch_handler routing
    jobject local = env->NewObject(listener_cls, ctor, handler_ptr, static_cast<jint>(1));
    if (local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(listener_cls);
        return nullptr;
    }

    jclass cb_cls = env->FindClass("android/widget/CompoundButton");
    if (cb_cls != nullptr) {
        jmethodID set_listener = env->GetMethodID(
            cb_cls, "setOnCheckedChangeListener",
            "(Landroid/widget/CompoundButton$OnCheckedChangeListener;)V");
        if (set_listener != nullptr) {
            env->CallVoidMethod(sw, set_listener, local);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(cb_cls);
    }

    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    env->DeleteLocalRef(listener_cls);
    return global;
}

} // namespace

switch_handler<platform::android>::switch_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env != nullptr) {
        native_ = make_switch(env, detail::get_activity());
    }
}

switch_handler<platform::android>::~switch_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (listener_ != nullptr) { env->DeleteGlobalRef(listener_); listener_ = nullptr; }
        if (native_   != nullptr) { env->DeleteGlobalRef(native_);   native_   = nullptr; }
    }
}

void switch_handler<platform::android>::apply_is_on(bool on) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    suppress_echo_ = true;
    switch_set_checked(env, native_, on);
    suppress_echo_ = false;
}

void switch_handler<platform::android>::map_is_on(basic_switch_& s) {
    bound_ = &s;
    apply_is_on(s.is_on.get());
    s.is_on.changed.subscribe(is_on_slot_, is_on_cb_);

    if (native_ != nullptr && listener_ == nullptr) {
        JNIEnv* env = detail::attach_current_thread();
        if (env != nullptr) {
            listener_ = install_checked_change_listener(env, native_,
                            reinterpret_cast<jlong>(this));
        }
    }
}

void switch_handler<platform::android>::on_native_checked_changed(bool checked) {
    if (suppress_echo_ || bound_ == nullptr) return;
    if (bound_->is_on.get() != checked) {
        bound_->is_on.set(checked);
    }
}

void android_switch_dispatch_checked_changed(switch_handler<platform::android>* h, bool checked) {
    if (h != nullptr) h->on_native_checked_changed(checked);
}

} // namespace mpapp::internal
// JNI trampoline moved into a shared dispatcher under
// src/handlers/android/compound_button_dispatch.cpp so multiple
// compound-basic_button handlers (basic_switch_, basic_check_box, …) can route through
// the same Java listener with a 'kind' discriminator.


// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_switch_ so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/internal/basic_switch_.hpp"

namespace {

jobject dispatch_switch(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_switch_*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_switch); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
