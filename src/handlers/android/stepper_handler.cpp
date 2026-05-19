// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android stepper handler implementation.

#include "mpapp/handlers/android/stepper_handler.hpp"

#if defined(__ANDROID__)

#include <algorithm>
#include <cmath>

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

namespace {

jobject make_number_picker(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/NumberPicker");
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

void np_set_min(JNIEnv* env, jobject np, int v) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/NumberPicker");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setMinValue", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(np, m, static_cast<jint>(v));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void np_set_max(JNIEnv* env, jobject np, int v) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/NumberPicker");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setMaxValue", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(np, m, static_cast<jint>(v));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void np_set_value(JNIEnv* env, jobject np, int v) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/NumberPicker");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setValue", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(np, m, static_cast<jint>(v));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

jobject install_number_picker_listener(JNIEnv* env, jobject np, jlong handler_ptr) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass listener_cls = env->FindClass("io/mpapp/MppNumberPickerListener");
    if (listener_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(listener_cls, "<init>", "(J)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(listener_cls); return nullptr; }
    jobject local = env->NewObject(listener_cls, ctor, handler_ptr);
    if (local == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(listener_cls); return nullptr; }
    jclass np_cls = env->FindClass("android/widget/NumberPicker");
    if (np_cls != nullptr) {
        jmethodID set_listener = env->GetMethodID(
            np_cls, "setOnValueChangedListener",
            "(Landroid/widget/NumberPicker$OnValueChangeListener;)V");
        if (set_listener != nullptr) {
            env->CallVoidMethod(np, set_listener, local);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(np_cls);
    }
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    env->DeleteLocalRef(listener_cls);
    return global;
}

} // namespace

stepper_handler<platform::android>::stepper_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env != nullptr) {
        native_ = make_number_picker(env, detail::get_activity());
    }
}

stepper_handler<platform::android>::~stepper_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (listener_ != nullptr) { env->DeleteGlobalRef(listener_); listener_ = nullptr; }
        if (native_   != nullptr) { env->DeleteGlobalRef(native_);   native_   = nullptr; }
    }
}

void stepper_handler<platform::android>::apply_range_and_value() {
    if (native_ == nullptr || bound_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    const double mn   = bound_->minimum.get();
    const double mx   = bound_->maximum.get();
    const double step = bound_->interval.get();
    const double v    = bound_->value.get();

    if (!(step > 0.0) || !(mx > mn)) return;

    const int max_idx = static_cast<int>(std::floor((mx - mn) / step + 0.5));
    int       idx     = static_cast<int>(std::floor((v - mn) / step + 0.5));
    idx = std::max(0, std::min(max_idx, idx));

    suppress_echo_ = true;
    np_set_min(env, native_, 0);
    np_set_max(env, native_, max_idx);
    np_set_value(env, native_, idx);
    suppress_echo_ = false;
}

void stepper_handler<platform::android>::map_value(stepper& s) {
    bound_ = &s;
    apply_range_and_value();
    s.value.changed.subscribe(value_slot_, value_cb_);

    if (native_ != nullptr && listener_ == nullptr) {
        JNIEnv* env = detail::attach_current_thread();
        if (env != nullptr) {
            listener_ = install_number_picker_listener(env, native_,
                            reinterpret_cast<jlong>(this));
        }
    }
}

void stepper_handler<platform::android>::map_minimum(stepper& s) {
    bound_ = &s;
    apply_range_and_value();
    s.minimum.changed.subscribe(minimum_slot_, minimum_cb_);
}
void stepper_handler<platform::android>::map_maximum(stepper& s) {
    bound_ = &s;
    apply_range_and_value();
    s.maximum.changed.subscribe(maximum_slot_, maximum_cb_);
}
void stepper_handler<platform::android>::map_interval(stepper& s) {
    bound_ = &s;
    apply_range_and_value();
    s.interval.changed.subscribe(interval_slot_, interval_cb_);
}

void stepper_handler<platform::android>::on_native_value_changed(int step_index) {
    if (suppress_echo_ || bound_ == nullptr) return;
    const double mn   = bound_->minimum.get();
    const double step = bound_->interval.get();
    if (!(step > 0.0)) return;
    const double v = mn + static_cast<double>(step_index) * step;
    if (bound_->value.get() != v) {
        bound_->value.set(v);
    }
}

void android_stepper_dispatch_value(stepper_handler<platform::android>* h, int step_index) {
    if (h != nullptr) h->on_native_value_changed(step_index);
}

} // namespace mpapp

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppNumberPickerListener_nativeDispatchValue(
    JNIEnv* /*env*/, jclass /*cls*/, jlong handler_ptr, jint value) {
    if (handler_ptr == 0) return;
    mpapp::android_stepper_dispatch_value(
        reinterpret_cast<mpapp::stepper_handler<mpapp::platform::android>*>(handler_ptr),
        static_cast<int>(value));
}

#endif // __ANDROID__
