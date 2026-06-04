// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Windows Win32 implementation of `mpapp::windows_battery`.
// windows.h is included here and nowhere else.

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "mpapp/essentials/windows/battery_windows.hpp"

namespace {

// ---- Translate SYSTEM_POWER_STATUS fields to mpapp enumerators ---------------

// BatteryLifePercent == 255 means "unknown".
constexpr BYTE kUnknownPercent = 255;

[[nodiscard]] double translate_charge_level(BYTE battery_life_percent) noexcept
{
    if (battery_life_percent == kUnknownPercent) {
        return -1.0;
    }
    return static_cast<double>(battery_life_percent) / 100.0;
}

// ACLineStatus values from the Win32 docs.
//   0 = Offline (on battery)
//   1 = Online  (on AC)
// 255 = Unknown
[[nodiscard]] mpapp::battery_power_source
translate_power_source(BYTE ac_line_status) noexcept
{
    switch (ac_line_status) {
        case 0:   return mpapp::battery_power_source::battery;
        case 1:   return mpapp::battery_power_source::ac;
        default:  return mpapp::battery_power_source::unknown;
    }
}

// BatteryFlag bit-field:
//   Bit 0 (0x01) = High     (> 66%)
//   Bit 1 (0x02) = Low      (< 33%)
//   Bit 2 (0x04) = Critical (< 5%)
//   Bit 3 (0x08) = Charging
//   Bit 7 (0x80) = No system battery / not present
//   255          = Unknown
[[nodiscard]] mpapp::battery_state
translate_state(BYTE ac_line_status, BYTE battery_flag) noexcept
{
    if (battery_flag == 255) {
        return mpapp::battery_state::unknown;
    }
    if (battery_flag & 0x80) {
        return mpapp::battery_state::not_present;
    }
    if (battery_flag & 0x08) {
        // Charging — check if actually full (AC + 100% reported as charging
        // with BatteryLifePercent == 100 in some drivers; we rely solely on
        // the flag here and let charge_level() convey the percentage).
        if (ac_line_status == 1 &&
            !(battery_flag & 0x01) &&
            !(battery_flag & 0x02) &&
            !(battery_flag & 0x04)) {
            // On AC and not low/critical — treat as full when the charging
            // flag is set together with the "high" flag and nothing else.
            // Some drivers set bit 0x08 | 0x01 when at 100%; be lenient.
        }
        return mpapp::battery_state::charging;
    }
    if (ac_line_status == 1) {
        // On AC power but not flagged as charging — fully charged.
        return mpapp::battery_state::full;
    }
    // On battery power and not charging.
    return mpapp::battery_state::discharging;
}

// SystemStatusFlag (Win8+):
//   Bit 0 (0x01) = Power saving is on (energy saver / battery saver active).
[[nodiscard]] mpapp::energy_saver_status
translate_energy_saver(BYTE system_status_flag) noexcept
{
    if (system_status_flag & 0x01) {
        return mpapp::energy_saver_status::on;
    }
    return mpapp::energy_saver_status::off;
}

} // anonymous namespace

namespace mpapp {

windows_battery::windows_battery()
{
    read_and_update();
}

double windows_battery::charge_level() const
{
    return info_.charge_level;
}

battery_state windows_battery::state() const
{
    return info_.state;
}

battery_power_source windows_battery::power_source() const
{
    return info_.power_source;
}

energy_saver_status windows_battery::energy_saver() const
{
    return energy_saver_;
}

void windows_battery::refresh()
{
    read_and_update();
}

bool windows_battery::read_and_update()
{
    SYSTEM_POWER_STATUS sps{};
    if (!::GetSystemPowerStatus(&sps)) {
        return false;
    }

    battery_info next{};
    next.charge_level  = translate_charge_level(sps.BatteryLifePercent);
    next.state         = translate_state(sps.ACLineStatus, sps.BatteryFlag);
    next.power_source  = translate_power_source(sps.ACLineStatus);

    const energy_saver_status next_es =
        translate_energy_saver(sps.SystemStatusFlag);

    const bool info_changed = !(next == info_);
    const bool es_changed   = (next_es != energy_saver_);

    info_         = next;
    energy_saver_ = next_es;

    if (info_changed) {
        battery_info_changed.emit(info_);
    }
    if (es_changed) {
        energy_saver_status_changed.emit(energy_saver_);
    }

    return true;
}

} // namespace mpapp
