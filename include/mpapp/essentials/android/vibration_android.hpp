// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android haptic backend — implements `mpapp::vibration`
// on top of `android.os.Vibrator`.
//
// This header is intentionally JNI-free: it declares the public class
// only, mirroring the project's Android handler pattern where <jni.h> and
// every JNI call are confined to the .cpp. The Context used to acquire the
// system Vibrator service is obtained through the Android JNI bridge
// (mpapp::detail::get_activity), so the public surface stays clean and
// macro-free for consumers and the DI container (RFC-0011).

#ifndef MPAPP_ESSENTIALS_ANDROID_VIBRATION_ANDROID_HPP
#define MPAPP_ESSENTIALS_ANDROID_VIBRATION_ANDROID_HPP

#include "mpapp/essentials/vibration.hpp"

namespace mpapp::essentials::android {

// Concrete `mpapp::vibration` backed by the platform Vibrator service.
//
// Uses Context.getSystemService(Context.VIBRATOR_SERVICE) to obtain an
// android.os.Vibrator, then drives it via VibrationEffect.createOneShot
// (API 26+). The Context comes from the JNI bridge's stored activity.
class android_vibration : public mpapp::vibration {
public:
    android_vibration() = default;
    ~android_vibration() override = default;

    android_vibration(const android_vibration&)            = delete;
    android_vibration& operator=(const android_vibration&) = delete;

    // Vibrate for the default duration (vibration_default_ms).
    void vibrate() override;

    // Vibrate for the specified duration in milliseconds.
    void vibrate(double milliseconds) override;

    // Cancel any active vibration.
    void cancel() override;
};

} // namespace mpapp::essentials::android

#endif // MPAPP_ESSENTIALS_ANDROID_VIBRATION_ANDROID_HPP
