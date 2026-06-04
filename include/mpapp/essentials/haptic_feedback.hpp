// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::haptic_feedback` — tactile feedback control. Counterpart to MAUI
// Essentials `HapticFeedback`. Abstract interface + an in-memory mock whose
// support flag is test-settable and whose last performed type and invocation
// count are observable. Real per-platform backends (Windows
// DeviceVibrationDevice, Android Vibrator, iOS UIFeedbackGenerator) implement
// the same interface and are injected via the DI container (RFC-0011).
// No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_HAPTIC_FEEDBACK_HPP
#define MPAPP_ESSENTIALS_HAPTIC_FEEDBACK_HPP

#include <cstdint>
#include <optional>
#include <string_view>

namespace mpapp {

// Mirrors MAUI's HapticFeedbackType.
enum class haptic_feedback_type : std::uint8_t {
    click      = 0,  // short, crisp tap (default button feedback)
    long_press = 1,  // longer, heavier vibration (hold/context-menu feedback)
};

// ---- to_string helper (constexpr, no macros) --------------------------------

[[nodiscard]] constexpr std::string_view to_string(haptic_feedback_type t) noexcept {
    switch (t) {
        case haptic_feedback_type::click:      return "click";
        case haptic_feedback_type::long_press: return "long_press";
        default:                               return "unknown";
    }
}

// ---- Abstract interface -----------------------------------------------------

class haptic_feedback {
public:
    virtual ~haptic_feedback() = default;

    // Trigger a haptic effect of the given type. If the device does not
    // support haptics the call must be a no-op (implementations must not throw).
    virtual void perform(haptic_feedback_type type) = 0;

    // Whether this device has usable haptic hardware.
    [[nodiscard]] virtual bool is_supported() const = 0;
};

// ---- Mock / in-memory implementation ----------------------------------------
//
// * `set_supported(false)` simulates a device without haptic hardware.
//   Calling `perform()` on such a mock is silently ignored (no state change)
//   and is NOT counted in `perform_count()`.
// * `last_type()` returns the type passed to the most recent accepted
//   `perform()` call, or `std::nullopt` if `perform()` has never been called
//   on a supported device.
// * `perform_count()` counts only accepted (supported) calls.

class mock_haptic_feedback final : public haptic_feedback {
public:
    explicit mock_haptic_feedback(bool supported = true) noexcept
        : supported_{ supported } {}

    // ---- haptic_feedback interface ------------------------------------------

    void perform(haptic_feedback_type type) override {
        if (!supported_) {
            return;
        }
        last_type_     = type;
        perform_count_ += 1;
    }

    [[nodiscard]] bool is_supported() const override { return supported_; }

    // ---- Test-control helpers -----------------------------------------------

    // Change the hardware-support flag at any time.
    void set_supported(bool s) noexcept { supported_ = s; }

    // The haptic_feedback_type passed to the most recent accepted perform(),
    // or std::nullopt if perform() has never succeeded.
    [[nodiscard]] std::optional<haptic_feedback_type> last_type() const noexcept {
        return last_type_;
    }

    // Number of times perform() was called on a supported device.
    [[nodiscard]] int perform_count() const noexcept { return perform_count_; }

    // Reset recorded state (last_type and perform_count) without changing the
    // supported flag.
    void reset() noexcept {
        last_type_     = std::nullopt;
        perform_count_ = 0;
    }

private:
    bool                                  supported_     = true;
    std::optional<haptic_feedback_type>   last_type_{};
    int                                   perform_count_ = 0;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_HAPTIC_FEEDBACK_HPP
