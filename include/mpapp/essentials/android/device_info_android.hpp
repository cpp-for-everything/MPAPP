// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::android_device_info` — Android device-info backend.
// Reads android.os.Build static fields (MODEL, MANUFACTURER) and
// Build.VERSION.RELEASE to fill an `mpapp::device_info` value type.
// No OS-specific headers (<jni.h>, android/…) appear here; all JNI
// details are confined to the .cpp translation unit, mirroring the
// Windows and Linux backends. No macros in the public API.

#ifndef MPAPP_ESSENTIALS_ANDROID_DEVICE_INFO_ANDROID_HPP
#define MPAPP_ESSENTIALS_ANDROID_DEVICE_INFO_ANDROID_HPP

#include "mpapp/essentials/device_info.hpp"

namespace mpapp {

// Factory: query android.os.Build static fields via JNI and return a
// filled `device_info` struct.
//
//   platform     — device_platform::android (always)
//   idiom        — device_idiom::phone (always for Android)
//   model        — android.os.Build.MODEL (UTF-8), e.g. "Pixel 8"
//                  Fallback: "Android" when the JVM is unavailable.
//   manufacturer — android.os.Build.MANUFACTURER (UTF-8), e.g. "Google"
//                  Fallback: "Unknown" when the JVM is unavailable.
//   version      — android.os.Build.VERSION.RELEASE (UTF-8), e.g. "14"
//                  Fallback: "unknown" when the JVM is unavailable.
//
// The JNIEnv / JavaVM is obtained from the JNI bridge
// (mpapp::detail::attach_current_thread). The host MainActivity is
// responsible for calling mpapp::detail::set_jni_vm / set_activity before
// any handler queries device info. A Context reference is NOT required
// in the constructor because android.os.Build fields are static constants
// that do not need a Context.
//
// All JNI local references are deleted before the function returns.
//
// This function is only defined when compiled for Android (__ANDROID__).
// The header is always parseable on any platform — the implementation
// guard lives in the .cpp.
[[nodiscard]] device_info android_device_info();

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_ANDROID_DEVICE_INFO_ANDROID_HPP
