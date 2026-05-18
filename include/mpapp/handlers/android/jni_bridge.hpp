// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android JNI bridge for the app-shell handlers.
//
// Centralises JavaVM / JNIEnv access used by every Android handler. The
// Android entry point (the user's `MainActivity` calling a native method)
// is responsible for calling `set_jni_vm` and `set_activity` once.

#ifndef MPAPP_HANDLERS_ANDROID_JNI_BRIDGE_HPP
#define MPAPP_HANDLERS_ANDROID_JNI_BRIDGE_HPP

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::detail {

// Stored from JNI_OnLoad. The Android handlers attach the current
// thread to it when they need a JNIEnv.
void      set_jni_vm(JavaVM* vm) noexcept;
JavaVM*   get_jni_vm() noexcept;

// Set from MainActivity's native init method. Held as a global ref.
void      set_activity(JNIEnv* env, jobject activity) noexcept;
jobject   get_activity() noexcept;

// Attach the current thread to the JVM and return a JNIEnv*. Safe to
// call repeatedly on the same thread.
JNIEnv*   attach_current_thread() noexcept;

} // namespace mpapp::detail

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_JNI_BRIDGE_HPP
