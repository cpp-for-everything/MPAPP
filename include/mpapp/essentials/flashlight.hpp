// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::flashlight` — torch / camera-flash control. Counterpart to MAUI
// Essentials `Flashlight`. Abstract interface + an in-memory mock whose
// support flag and on/off state are test-settable. Real per-platform
// backends (Windows DeviceInformation, Android CameraManager, iOS
// AVCaptureDevice) implement the same interface and are injected via
// the DI container (RFC-0011). No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_FLASHLIGHT_HPP
#define MPAPP_ESSENTIALS_FLASHLIGHT_HPP

#include <cstdint>

namespace mpapp {

class flashlight {
public:
    virtual ~flashlight() = default;

    // Turn the torch on. If the device does not support a flashlight the
    // call is a no-op (implementations must not throw).
    virtual void turn_on() = 0;

    // Turn the torch off.
    virtual void turn_off() = 0;

    // Whether the torch is currently on.
    [[nodiscard]] virtual bool is_on() const = 0;

    // Whether this device has a usable flashlight at all.
    [[nodiscard]] virtual bool is_supported() const = 0;
};

// Mock / in-memory implementation.
//
// * `set_supported(false)` simulates a device without a torch. Calling
//   `turn_on()` on such a mock is silently ignored (no state change) and
//   increments `not_supported_attempts()`.
// * `set_supported(true)` re-enables normal operation; any accumulated
//   not-supported attempt count is preserved.
// * `turn_on()` / `turn_off()` toggle `is_on()` exactly as expected on a
//   real device (idempotent: calling `turn_on()` twice leaves the light on).
class mock_flashlight final : public flashlight {
public:
    explicit mock_flashlight(bool supported = true) noexcept
        : supported_{ supported } {}

    // ---- flashlight interface -------------------------------------------

    void turn_on() override {
        if (!supported_) {
            ++not_supported_attempts_;
            return;
        }
        on_ = true;
    }

    void turn_off() override {
        on_ = false;
    }

    [[nodiscard]] bool is_on() const override { return on_; }

    [[nodiscard]] bool is_supported() const override { return supported_; }

    // ---- Test-control helpers ------------------------------------------

    // Change the hardware-support flag at any time.
    void set_supported(bool s) noexcept { supported_ = s; }

    // How many times turn_on() was called while !is_supported().
    [[nodiscard]] int not_supported_attempts() const noexcept {
        return not_supported_attempts_;
    }

private:
    bool on_                    = false;
    bool supported_             = true;
    int  not_supported_attempts_ = 0;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_FLASHLIGHT_HPP
