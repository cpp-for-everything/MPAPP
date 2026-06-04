// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::windows_battery` — Windows Win32 backend for `mpapp::battery`.
// Uses GetSystemPowerStatus (SYSTEM_POWER_STATUS) to read charge level,
// charging state, power source, and energy-saver status.
// windows.h is confined to the .cpp; this header is platform-neutral.

#ifndef MPAPP_ESSENTIALS_WINDOWS_BATTERY_WINDOWS_HPP
#define MPAPP_ESSENTIALS_WINDOWS_BATTERY_WINDOWS_HPP

#include <cstdint>

#include "../../essentials/battery.hpp"

namespace mpapp {

// Windows Win32 implementation of `mpapp::battery`.
//
// Reads SYSTEM_POWER_STATUS via GetSystemPowerStatus on construction and on
// each call to refresh(). refresh() emits battery_info_changed and/or
// energy_saver_status_changed whenever the snapshot differs from the
// previously cached one.
//
// Thread safety: not thread-safe. Call from a single thread or guard
// externally.
class windows_battery final : public battery {
public:
    // Constructs and performs an initial read of the system power status.
    windows_battery();

    windows_battery(const windows_battery&)            = delete;
    windows_battery& operator=(const windows_battery&) = delete;
    windows_battery(windows_battery&&)                 = delete;
    windows_battery& operator=(windows_battery&&)      = delete;

    ~windows_battery() override = default;

    // ---- battery interface --------------------------------------------------

    // Returns the current charge level in [0.0, 1.0].
    // Returns -1.0 when the level is unknown (BatteryLifePercent == 255).
    [[nodiscard]] double              charge_level() const override;
    [[nodiscard]] battery_state       state()        const override;
    [[nodiscard]] battery_power_source power_source() const override;
    [[nodiscard]] energy_saver_status  energy_saver() const override;

    // Re-reads SYSTEM_POWER_STATUS. Emits battery_info_changed if
    // charge_level, state, or power_source changed. Emits
    // energy_saver_status_changed if the energy-saver flag changed.
    void refresh();

private:
    // Cached snapshot — updated by the last successful read.
    battery_info         info_{};
    energy_saver_status  energy_saver_{ energy_saver_status::unknown };

    // Performs a Win32 read and updates info_ / energy_saver_.
    // Returns true when GetSystemPowerStatus succeeded.
    bool read_and_update();
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_WINDOWS_BATTERY_WINDOWS_HPP
