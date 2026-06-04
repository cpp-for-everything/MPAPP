// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::android_flashlight` — Android torch/camera-flash backend.
// Implements `mpapp::flashlight` using android.hardware.camera2.CameraManager
// (API 23+), reached through the app Context obtained from the JNI bridge
// (mpapp::detail::get_activity()). All JNI details (<jni.h>, FindClass,
// CallObjectMethod, DeleteLocalRef, AttachCurrentThread) are confined to
// the .cpp translation unit; this header stays JNI-free, mirroring the
// clipboard and vibration Android backends. No macros in the public API.

#ifndef MPAPP_ESSENTIALS_ANDROID_FLASHLIGHT_ANDROID_HPP
#define MPAPP_ESSENTIALS_ANDROID_FLASHLIGHT_ANDROID_HPP

#include "../../essentials/flashlight.hpp"

namespace mpapp {

// Android flashlight backend. Implements `mpapp::flashlight` via
// android.hardware.camera2.CameraManager.setTorchMode() (API 23+).
//
// The Context is taken from the JNI bridge (detail::get_activity()), which
// the host MainActivity sets once during native init. No Context is required
// in the constructor.
//
// is_supported() -> Context.getPackageManager().hasSystemFeature(
//                       PackageManager.FEATURE_CAMERA_FLASH)
// turn_on()      -> CameraManager.setTorchMode(cameraId, true);
//                   no-op (does not throw) when !is_supported()
// turn_off()     -> CameraManager.setTorchMode(cameraId, false)
// is_on()        -> tracks the last successful setTorchMode call
//
// All JNI local refs are deleted before each method returns. The first
// back-facing camera ID reported by CameraManager.getCameraIdList() is used.
class android_flashlight final : public flashlight {
public:
    android_flashlight()  = default;
    ~android_flashlight() = default;

    android_flashlight(const android_flashlight&)            = delete;
    android_flashlight& operator=(const android_flashlight&) = delete;
    android_flashlight(android_flashlight&&)                 = delete;
    android_flashlight& operator=(android_flashlight&&)      = delete;

    // Turn the torch on via CameraManager.setTorchMode(id, true).
    // No-op (no throw) when is_supported() is false or JNI is unavailable.
    void turn_on() override;

    // Turn the torch off via CameraManager.setTorchMode(id, false).
    void turn_off() override;

    // Returns true after a successful turn_on(); false after turn_off() or
    // construction (state is not queried from the OS, it is tracked locally).
    [[nodiscard]] bool is_on() const override;

    // Returns true when the device reports FEATURE_CAMERA_FLASH via
    // PackageManager.hasSystemFeature(). Returns false when JNI is
    // unavailable or the Context has not been set in the bridge.
    [[nodiscard]] bool is_supported() const override;

private:
    bool on_{ false };
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_ANDROID_FLASHLIGHT_ANDROID_HPP
