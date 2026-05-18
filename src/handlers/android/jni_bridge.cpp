// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android JNI bridge implementation.

#include "mpapp/handlers/android/jni_bridge.hpp"

#if defined(__ANDROID__)

namespace mpapp::detail {

namespace {
JavaVM* g_vm        = nullptr;
jobject g_activity  = nullptr;  // global ref
}

void set_jni_vm(JavaVM* vm) noexcept { g_vm = vm; }
JavaVM* get_jni_vm() noexcept { return g_vm; }

void set_activity(JNIEnv* env, jobject activity) noexcept {
    if (g_activity != nullptr) {
        env->DeleteGlobalRef(g_activity);
    }
    g_activity = env->NewGlobalRef(activity);
}

jobject get_activity() noexcept { return g_activity; }

JNIEnv* attach_current_thread() noexcept {
    if (g_vm == nullptr) return nullptr;
    JNIEnv* env = nullptr;
    if (g_vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_OK) {
        return env;
    }
    if (g_vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
        return env;
    }
    return nullptr;
}

} // namespace mpapp::detail

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/) {
    mpapp::detail::set_jni_vm(vm);
    return JNI_VERSION_1_6;
}

#endif // __ANDROID__
