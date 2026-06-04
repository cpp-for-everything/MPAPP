// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::device_display` — screen metrics and orientation for the main
// display.  Counterpart to MAUI Essentials `DeviceDisplay`.  Abstract
// interface + an in-memory mock whose state is settable so tests can drive
// `main_display_info_changed`.  Real per-platform backends (Windows
// DisplayInformation, Linux XRandR, Android WindowManager) implement the
// same interface and are injected via the DI container (RFC-0011).
// No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_DEVICE_DISPLAY_HPP
#define MPAPP_ESSENTIALS_DEVICE_DISPLAY_HPP

#include <cstdint>
#include <string_view>

#include "../signal.hpp"

namespace mpapp {

// ---- Enumerations ----------------------------------------------------------

enum class display_orientation : std::uint8_t {
    unknown   = 0,
    portrait  = 1,
    landscape = 2,
};

enum class display_rotation : std::uint8_t {
    rotation_0   = 0,
    rotation_90  = 1,
    rotation_180 = 2,
    rotation_270 = 3,
};

// ---- to_string helpers (constexpr, no macros) ------------------------------

[[nodiscard]] constexpr std::string_view to_string(display_orientation o) noexcept {
    switch (o) {
        case display_orientation::portrait:  return "portrait";
        case display_orientation::landscape: return "landscape";
        default:                             return "unknown";
    }
}

[[nodiscard]] constexpr std::string_view to_string(display_rotation r) noexcept {
    switch (r) {
        case display_rotation::rotation_0:   return "rotation_0";
        case display_rotation::rotation_90:  return "rotation_90";
        case display_rotation::rotation_180: return "rotation_180";
        case display_rotation::rotation_270: return "rotation_270";
        default:                             return "rotation_0";
    }
}

// ---- Value type ------------------------------------------------------------

struct display_info {
    double             width       = 0.0;
    double             height      = 0.0;
    double             density     = 1.0;
    double             rate        = 60.0;
    display_orientation orientation = display_orientation::unknown;
    display_rotation   rotation    = display_rotation::rotation_0;

    bool operator==(const display_info&) const = default;
};

// ---- Abstract interface ----------------------------------------------------

class device_display {
public:
    virtual ~device_display() = default;

    [[nodiscard]] virtual display_info main_display_info() const = 0;

    [[nodiscard]] virtual bool keep_screen_on() const = 0;
    virtual void set_keep_screen_on(bool value)       = 0;

    // Fires whenever the main display metrics change.
    mpapp::signal<display_info> main_display_info_changed{};
};

// ---- Mock / in-memory implementation ---------------------------------------

// State is fully settable; calling set_main_display_info with a different
// value emits the change signal.
class mock_device_display final : public device_display {
public:
    explicit mock_device_display(display_info initial = display_info{})
        : info_{ initial } {}

    [[nodiscard]] display_info main_display_info() const override {
        return info_;
    }

    // Setting the same value is a no-op (no signal fired).
    void set_main_display_info(const display_info& info) {
        if (info == info_) {
            return;
        }
        info_ = info;
        main_display_info_changed.emit(info_);
    }

    [[nodiscard]] bool keep_screen_on() const override {
        return keep_screen_on_;
    }

    void set_keep_screen_on(bool value) override {
        keep_screen_on_ = value;
    }

private:
    display_info info_{};
    bool         keep_screen_on_ = false;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_DEVICE_DISPLAY_HPP
