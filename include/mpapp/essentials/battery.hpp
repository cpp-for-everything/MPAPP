// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::battery` — battery level, charge state and energy-saver status.
// Counterpart to MAUI Essentials `Battery`. Abstract interface + an
// in-memory mock whose state is fully test-settable. Real per-platform
// backends (Windows SystemPowerStatus / EnergySaver, Linux UPower,
// Android BatteryManager, iOS UIDevice) implement the same interface and
// are injected via the DI container (RFC-0011). No macros; header-only.

#ifndef MPAPP_ESSENTIALS_BATTERY_HPP
#define MPAPP_ESSENTIALS_BATTERY_HPP

#include <cstdint>
#include <string_view>

#include "../signal.hpp"

namespace mpapp {

// Current charging state — mirrors MAUI BatteryState.
enum class battery_state : std::uint8_t {
    unknown      = 0,
    charging     = 1,
    discharging  = 2,
    full         = 3,
    not_charging = 4,
    not_present  = 5,
};

// Source of current power — mirrors MAUI BatteryPowerSource.
enum class battery_power_source : std::uint8_t {
    unknown  = 0,
    battery  = 1,
    ac       = 2,
    usb      = 3,
    wireless = 4,
};

// Whether the OS energy-saver / low-power mode is active.
enum class energy_saver_status : std::uint8_t {
    unknown = 0,
    on      = 1,
    off     = 2,
};

[[nodiscard]] constexpr std::string_view to_string(battery_state s) noexcept {
    switch (s) {
        case battery_state::charging:     return "charging";
        case battery_state::discharging:  return "discharging";
        case battery_state::full:         return "full";
        case battery_state::not_charging: return "not_charging";
        case battery_state::not_present:  return "not_present";
        case battery_state::unknown:      return "unknown";
        default:                          return "?";
    }
}

[[nodiscard]] constexpr std::string_view to_string(battery_power_source ps) noexcept {
    switch (ps) {
        case battery_power_source::battery:  return "battery";
        case battery_power_source::ac:       return "ac";
        case battery_power_source::usb:      return "usb";
        case battery_power_source::wireless: return "wireless";
        case battery_power_source::unknown:  return "unknown";
        default:                             return "?";
    }
}

[[nodiscard]] constexpr std::string_view to_string(energy_saver_status es) noexcept {
    switch (es) {
        case energy_saver_status::on:      return "on";
        case energy_saver_status::off:     return "off";
        case energy_saver_status::unknown: return "unknown";
        default:                           return "?";
    }
}

// Payload emitted by battery_info_changed. Groups state + level + source so
// subscribers receive a consistent snapshot.
struct battery_info {
    double              charge_level  = 0.0;   // 0.0..1.0 (negative if unknown)
    battery_state       state         = battery_state::unknown;
    battery_power_source power_source = battery_power_source::unknown;

    bool operator==(const battery_info&) const = default;
};

// Abstract interface.
class battery {
public:
    virtual ~battery() = default;

    // Current charge level in the range [0, 1]. Returns a negative value
    // when the level is not available (e.g. no battery present).
    [[nodiscard]] virtual double charge_level() const = 0;

    [[nodiscard]] virtual battery_state       state()              const = 0;
    [[nodiscard]] virtual battery_power_source power_source()      const = 0;
    [[nodiscard]] virtual energy_saver_status  energy_saver()      const = 0;

    // Fires whenever charge_level, state, or power_source changes.
    mpapp::signal<battery_info> battery_info_changed{};

    // Fires whenever the energy-saver mode is toggled.
    mpapp::signal<energy_saver_status> energy_saver_status_changed{};
};

// Mock / in-memory implementation.
//
// * All four properties are independently settable via set_*() helpers.
// * `set_charge_level()`, `set_state()`, `set_power_source()` each write
//   their value and, if the overall battery_info snapshot changed, emit
//   battery_info_changed with the new snapshot.
// * `set_energy_saver()` emits energy_saver_status_changed when the value
//   differs from the current one.
// * `set_supported(false)` simulates a device with no battery; charge_level()
//   returns -1.0, state() returns battery_state::not_present, power_source()
//   returns battery_power_source::unknown.
class mock_battery final : public battery {
public:
    explicit mock_battery(bool supported = true) noexcept
        : supported_{ supported } {}

    // ---- battery interface ------------------------------------------------

    [[nodiscard]] double charge_level() const override {
        if (!supported_) return -1.0;
        return info_.charge_level;
    }

    [[nodiscard]] battery_state state() const override {
        if (!supported_) return battery_state::not_present;
        return info_.state;
    }

    [[nodiscard]] battery_power_source power_source() const override {
        if (!supported_) return battery_power_source::unknown;
        return info_.power_source;
    }

    [[nodiscard]] energy_saver_status energy_saver() const override {
        return energy_saver_;
    }

    // ---- Test-control helpers --------------------------------------------

    void set_supported(bool s) noexcept { supported_ = s; }
    [[nodiscard]] bool is_supported() const noexcept { return supported_; }

    // Set charge level (clamped is the caller's responsibility; mock accepts
    // any value so tests can verify out-of-range behaviour if desired).
    void set_charge_level(double level) {
        if (!supported_) return;
        battery_info next = info_;
        next.charge_level = level;
        apply_info(next);
    }

    void set_state(battery_state s) {
        if (!supported_) return;
        battery_info next = info_;
        next.state = s;
        apply_info(next);
    }

    void set_power_source(battery_power_source ps) {
        if (!supported_) return;
        battery_info next = info_;
        next.power_source = ps;
        apply_info(next);
    }

    void set_energy_saver(energy_saver_status es) {
        if (es == energy_saver_) return;
        energy_saver_ = es;
        energy_saver_status_changed.emit(es);
    }

private:
    void apply_info(const battery_info& next) {
        if (next == info_) return;
        info_ = next;
        battery_info_changed.emit(info_);
    }

    bool                  supported_     = true;
    battery_info          info_{};
    energy_saver_status   energy_saver_  = energy_saver_status::unknown;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_BATTERY_HPP
