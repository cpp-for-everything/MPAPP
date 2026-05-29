// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_slider handler implementation.

#include "mpapp/handlers/android/slider_handler.hpp"

#if defined(__ANDROID__)

#include <algorithm>
#include <cmath>

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp::internal {

namespace {

jobject make_seek_bar(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/SeekBar");
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

void seek_bar_set_max(JNIEnv* env, jobject sb, int max) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ProgressBar");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setMax", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(sb, m, static_cast<jint>(max));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void seek_bar_set_progress(JNIEnv* env, jobject sb, int progress) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ProgressBar");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setProgress", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(sb, m, static_cast<jint>(progress));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

jobject install_seek_bar_listener(JNIEnv* env, jobject sb, jlong handler_ptr) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass listener_cls = env->FindClass("io/mpapp/MppSeekBarChangeListener");
    if (listener_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(listener_cls, "<init>", "(J)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(listener_cls); return nullptr; }
    jobject local = env->NewObject(listener_cls, ctor, handler_ptr);
    if (local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(listener_cls);
        return nullptr;
    }
    jclass sb_cls = env->FindClass("android/widget/SeekBar");
    if (sb_cls != nullptr) {
        jmethodID set_listener = env->GetMethodID(
            sb_cls, "setOnSeekBarChangeListener",
            "(Landroid/widget/SeekBar$OnSeekBarChangeListener;)V");
        if (set_listener != nullptr) {
            env->CallVoidMethod(sb, set_listener, local);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(sb_cls);
    }
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    env->DeleteLocalRef(listener_cls);
    return global;
}

double clamp(double v, double lo, double hi) {
    return std::max(lo, std::min(hi, v));
}

} // namespace

slider_handler<platform::android>::slider_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env != nullptr) {
        native_ = make_seek_bar(env, detail::get_activity());
        if (native_ != nullptr) {
            seek_bar_set_max(env, native_, kSeekResolution);
        }
    }
}

slider_handler<platform::android>::~slider_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (listener_ != nullptr) { env->DeleteGlobalRef(listener_); listener_ = nullptr; }
        if (native_   != nullptr) { env->DeleteGlobalRef(native_);   native_   = nullptr; }
    }
}

void slider_handler<platform::android>::update_native_progress_from_value() {
    if (native_ == nullptr || bound_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    const double mn = bound_->minimum.get();
    const double mx = bound_->maximum.get();
    const double v  = bound_->value.get();
    int prog = 0;
    if (mx > mn) {
        const double norm = (v - mn) / (mx - mn);
        prog = static_cast<int>(std::round(clamp(norm, 0.0, 1.0) * kSeekResolution));
    }
    suppress_echo_ = true;
    seek_bar_set_progress(env, native_, prog);
    suppress_echo_ = false;
}

void slider_handler<platform::android>::apply_value(double /*v*/) {
    update_native_progress_from_value();
}
void slider_handler<platform::android>::apply_minimum(double /*v*/) {
    update_native_progress_from_value();
}
void slider_handler<platform::android>::apply_maximum(double /*v*/) {
    update_native_progress_from_value();
}

void slider_handler<platform::android>::map_value(basic_slider& s) {
    bound_ = &s;
    update_native_progress_from_value();
    s.value.changed.subscribe(value_slot_, value_cb_);

    if (native_ != nullptr && listener_ == nullptr) {
        JNIEnv* env = detail::attach_current_thread();
        if (env != nullptr) {
            listener_ = install_seek_bar_listener(env, native_,
                            reinterpret_cast<jlong>(this));
        }
    }
}

void slider_handler<platform::android>::map_minimum(basic_slider& s) {
    bound_ = &s;
    update_native_progress_from_value();
    s.minimum.changed.subscribe(minimum_slot_, minimum_cb_);
}

void slider_handler<platform::android>::map_maximum(basic_slider& s) {
    bound_ = &s;
    update_native_progress_from_value();
    s.maximum.changed.subscribe(maximum_slot_, maximum_cb_);
}

void slider_handler<platform::android>::on_native_progress_changed(int progress, int max) {
    if (suppress_echo_ || bound_ == nullptr || max <= 0) return;
    const double mn = bound_->minimum.get();
    const double mx = bound_->maximum.get();
    if (!(mx > mn)) return;
    const double norm = static_cast<double>(progress) / static_cast<double>(max);
    const double v    = mn + clamp(norm, 0.0, 1.0) * (mx - mn);
    if (bound_->value.get() != v) {
        bound_->value.set(v);
    }
}

void android_slider_dispatch_progress(slider_handler<platform::android>* h, int progress, int max) {
    if (h != nullptr) h->on_native_progress_changed(progress, max);
}

} // namespace mpapp::internal
extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppSeekBarChangeListener_nativeDispatchProgress(
    JNIEnv* /*env*/, jclass /*cls*/, jlong handler_ptr, jint progress, jint max) {
    if (handler_ptr == 0) return;
    mpapp::internal::android_slider_dispatch_progress(
        reinterpret_cast<mpapp::internal::slider_handler<mpapp::platform::android>*>(handler_ptr),
        static_cast<int>(progress), static_cast<int>(max));
}


// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_slider so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/internal/basic_slider.hpp"

namespace {

jobject dispatch_slider(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_slider*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_slider); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
