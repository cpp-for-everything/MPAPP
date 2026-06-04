// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android haptic backend implementation.
//
// Confines <jni.h> and every JNI call here, mirroring the project's Android
// handler pattern (jni_bridge for JNIEnv acquisition, the
// `#if defined(__ANDROID__)` guard). The header stays JNI-free.

#include "mpapp/essentials/android/vibration_android.hpp"

#if defined(__ANDROID__)

#include <jni.h>

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp::essentials::android {

namespace {

// Resolve android.os.Vibrator via Context.getSystemService(VIBRATOR_SERVICE).
// Returns a local ref (caller owns) or nullptr. Clears any pending exception.
jobject acquire_vibrator(JNIEnv* env, jobject context) {
    if (env == nullptr || context == nullptr) return nullptr;
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass context_cls = env->FindClass("android/content/Context");
    if (context_cls == nullptr) { env->ExceptionClear(); return nullptr; }

    // Read the static String constant Context.VIBRATOR_SERVICE.
    jfieldID svc_field = env->GetStaticFieldID(
        context_cls, "VIBRATOR_SERVICE", "Ljava/lang/String;");
    if (svc_field == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(context_cls);
        return nullptr;
    }
    jobject service_name = env->GetStaticObjectField(context_cls, svc_field);

    jmethodID get_service = env->GetMethodID(
        context_cls, "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;");
    env->DeleteLocalRef(context_cls);
    if (get_service == nullptr) {
        env->ExceptionClear();
        if (service_name != nullptr) env->DeleteLocalRef(service_name);
        return nullptr;
    }

    jobject vibrator = env->CallObjectMethod(context, get_service, service_name);
    if (service_name != nullptr) env->DeleteLocalRef(service_name);
    if (env->ExceptionCheck() || vibrator == nullptr) {
        env->ExceptionClear();
        if (vibrator != nullptr) env->DeleteLocalRef(vibrator);
        return nullptr;
    }
    return vibrator;
}

} // namespace

void android_vibration::vibrate(double milliseconds) {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    jobject vibrator = acquire_vibrator(env, detail::get_activity());
    if (vibrator == nullptr) return;

    // VibrationEffect.createOneShot(long, int) -> VibrationEffect (API 26+).
    jclass effect_cls = env->FindClass("android/os/VibrationEffect");
    if (effect_cls == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(vibrator);
        return;
    }

    jint amplitude = static_cast<jint>(-1);  // VibrationEffect.DEFAULT_AMPLITUDE
    jfieldID amp_field = env->GetStaticFieldID(
        effect_cls, "DEFAULT_AMPLITUDE", "I");
    if (amp_field != nullptr) {
        amplitude = env->GetStaticIntField(effect_cls, amp_field);
    } else {
        env->ExceptionClear();
    }

    jmethodID create_one_shot = env->GetStaticMethodID(
        effect_cls, "createOneShot", "(JI)Landroid/os/VibrationEffect;");
    if (create_one_shot == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(effect_cls);
        env->DeleteLocalRef(vibrator);
        return;
    }

    jlong ms = static_cast<jlong>(milliseconds);
    jobject effect = env->CallStaticObjectMethod(
        effect_cls, create_one_shot, ms, amplitude);
    env->DeleteLocalRef(effect_cls);
    if (env->ExceptionCheck() || effect == nullptr) {
        env->ExceptionClear();
        if (effect != nullptr) env->DeleteLocalRef(effect);
        env->DeleteLocalRef(vibrator);
        return;
    }

    // Vibrator.vibrate(VibrationEffect).
    jclass vibrator_cls = env->GetObjectClass(vibrator);
    if (vibrator_cls != nullptr) {
        jmethodID vibrate_effect = env->GetMethodID(
            vibrator_cls, "vibrate", "(Landroid/os/VibrationEffect;)V");
        if (vibrate_effect != nullptr) {
            env->CallVoidMethod(vibrator, vibrate_effect, effect);
            if (env->ExceptionCheck()) env->ExceptionClear();
        } else {
            env->ExceptionClear();
        }
        env->DeleteLocalRef(vibrator_cls);
    }

    env->DeleteLocalRef(effect);
    env->DeleteLocalRef(vibrator);
}

void android_vibration::vibrate() {
    vibrate(vibration_default_ms);
}

void android_vibration::cancel() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    jobject vibrator = acquire_vibrator(env, detail::get_activity());
    if (vibrator == nullptr) return;

    jclass vibrator_cls = env->GetObjectClass(vibrator);
    if (vibrator_cls != nullptr) {
        jmethodID cancel_id = env->GetMethodID(vibrator_cls, "cancel", "()V");
        if (cancel_id != nullptr) {
            env->CallVoidMethod(vibrator, cancel_id);
            if (env->ExceptionCheck()) env->ExceptionClear();
        } else {
            env->ExceptionClear();
        }
        env->DeleteLocalRef(vibrator_cls);
    }

    env->DeleteLocalRef(vibrator);
}

} // namespace mpapp::essentials::android

#endif // __ANDROID__
