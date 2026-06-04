// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::vibration` — haptic feedback control. Counterpart to MAUI
// Essentials `Vibration`. Abstract interface + a mock implementation
// whose recorded state is inspectable so tests can drive haptic
// interactions. Real per-platform backends (Android Vibrator,
// iOS UIImpactFeedbackGenerator, Windows actuator) implement the same
// interface and are injected via the DI container (RFC-0011). No macros.

#ifndef MPAPP_ESSENTIALS_VIBRATION_HPP
#define MPAPP_ESSENTIALS_VIBRATION_HPP

#include <cstdint>

namespace mpapp {

// Default vibration duration in milliseconds when no duration is specified.
inline constexpr double vibration_default_ms = 500.0;

class vibration {
public:
    virtual ~vibration() = default;

    // Vibrate for the default duration (vibration_default_ms).
    virtual void vibrate() = 0;

    // Vibrate for the specified duration in milliseconds.
    virtual void vibrate(double milliseconds) = 0;

    // Cancel any active vibration.
    virtual void cancel() = 0;
};

// Mock / in-memory implementation: records calls so tests can inspect them.
class mock_vibration final : public vibration {
public:
    mock_vibration() = default;

    void vibrate() override {
        vibrate(vibration_default_ms);
    }

    void vibrate(double milliseconds) override {
        last_duration_ms_ = milliseconds;
        ++vibrate_count_;
        canceled_ = false;
    }

    void cancel() override {
        canceled_ = true;
    }

    // ---- Inspection helpers ------------------------------------------------

    // Returns the duration (ms) passed to the most recent vibrate() call.
    // Zero if vibrate() has never been called.
    [[nodiscard]] double last_duration_ms() const noexcept {
        return last_duration_ms_;
    }

    // Returns the total number of times vibrate() (either overload) was called.
    [[nodiscard]] int vibrate_count() const noexcept {
        return vibrate_count_;
    }

    // Returns true if cancel() was the last action, false otherwise.
    [[nodiscard]] bool was_canceled() const noexcept {
        return canceled_;
    }

    // Reset recorded state.
    void reset() noexcept {
        last_duration_ms_ = 0.0;
        vibrate_count_    = 0;
        canceled_         = false;
    }

private:
    double last_duration_ms_ = 0.0;
    int    vibrate_count_    = 0;
    bool   canceled_         = false;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_VIBRATION_HPP
