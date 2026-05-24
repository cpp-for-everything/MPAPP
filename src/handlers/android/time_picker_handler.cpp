// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_time_picker handler implementation.

#include "mpapp/handlers/android/time_picker_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp::internal {

namespace {

jobject make_time_picker(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TimePicker");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck() || local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(cls);
        return nullptr;
    }
    // 24-hour mode by default.
    jmethodID set_24h = env->GetMethodID(cls, "setIs24HourView", "(Ljava/lang/Boolean;)V");
    if (set_24h != nullptr) {
        jclass bool_cls = env->FindClass("java/lang/Boolean");
        if (bool_cls != nullptr) {
            jmethodID value_of = env->GetStaticMethodID(
                bool_cls, "valueOf", "(Z)Ljava/lang/Boolean;");
            if (value_of != nullptr) {
                jobject true_box = env->CallStaticObjectMethod(bool_cls, value_of, JNI_TRUE);
                env->CallVoidMethod(local, set_24h, true_box);
                if (env->ExceptionCheck()) env->ExceptionClear();
                env->DeleteLocalRef(true_box);
            }
            env->DeleteLocalRef(bool_cls);
        }
    }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

} // namespace

time_picker_handler<platform::android>::time_picker_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_time_picker(env, detail::get_activity());
}

time_picker_handler<platform::android>::~time_picker_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void time_picker_handler<platform::android>::apply_time(const time_value& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TimePicker");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    // setHour + setMinute on API 23+; fall back to setCurrentHour/Minute
    // for older runtimes.
    jmethodID set_hour   = env->GetMethodID(cls, "setHour",   "(I)V");
    jmethodID set_minute = env->GetMethodID(cls, "setMinute", "(I)V");
    if (set_hour != nullptr && set_minute != nullptr) {
        env->CallVoidMethod(native_, set_hour,   static_cast<jint>(v.hour));
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->CallVoidMethod(native_, set_minute, static_cast<jint>(v.minute));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void time_picker_handler<platform::android>::map_time(basic_time_picker& p) {
    apply_time(p.time.get());
    p.time.changed.subscribe(time_slot_, time_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_time_picker so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/internal/basic_time_picker.hpp"

namespace {

jobject dispatch_time_picker(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_time_picker*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_time_picker); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
