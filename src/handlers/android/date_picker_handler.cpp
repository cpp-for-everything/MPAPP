// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android date_picker handler implementation.

#include "mpapp/handlers/android/date_picker_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

namespace {

jobject make_date_picker(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/DatePicker");
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

} // namespace

date_picker_handler<platform::android>::date_picker_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_date_picker(env, detail::get_activity());
}

date_picker_handler<platform::android>::~date_picker_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void date_picker_handler<platform::android>::apply_date(const date_value& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/DatePicker");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    // updateDate(int year, int monthOfYear, int dayOfMonth) — Android's
    // month is 0-indexed.
    jmethodID upd = env->GetMethodID(cls, "updateDate", "(III)V");
    if (upd != nullptr) {
        env->CallVoidMethod(native_, upd,
                            static_cast<jint>(v.year),
                            static_cast<jint>(v.month - 1),
                            static_cast<jint>(v.day));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void date_picker_handler<platform::android>::map_date(date_picker& p) {
    apply_date(p.date.get());
    p.date.changed.subscribe(date_slot_, date_cb_);
}

} // namespace mpapp


// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register date_picker so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/date_picker.hpp"

namespace {

jobject dispatch_date_picker(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::date_picker*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_date_picker); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
