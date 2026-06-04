// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::linux_battery` — UPower/GDBus backend for Linux.
// Implements `mpapp::battery` using the org.freedesktop.UPower D-Bus service.
// All GIO/GLib/GDBus headers are confined to the .cpp translation unit;
// this header exposes only the class declaration.

#ifndef MPAPP_ESSENTIALS_LINUX_BATTERY_LINUX_HPP
#define MPAPP_ESSENTIALS_LINUX_BATTERY_LINUX_HPP

#include <cstdint>

#include "../../essentials/battery.hpp"

namespace mpapp {

// GDBus/UPower backend. Implements `mpapp::battery` via
// org.freedesktop.UPower on the system bus, reading the DisplayDevice proxy.
//
// charge_level() — reads the UPower Percentage property [0..100] scaled to
//                  [0.0..1.0]. Returns -1.0 when unavailable.
// state()        — maps the UPower State uint to battery_state:
//                    1 (Charging)    -> charging
//                    2 (Discharging) -> discharging
//                    4 (Full)        -> full
//                    5 (Pending-charge) -> charging
//                    6 (Pending-discharge) -> discharging
//                    other           -> unknown
// power_source() — derived from the UPower OnBattery property:
//                    false -> battery_power_source::ac
//                    true  -> battery_power_source::battery
// energy_saver() — always returns energy_saver_status::unknown (no UPower API)
//
// refresh() re-reads all properties and emits battery_info_changed /
// energy_saver_status_changed if values have changed.
//
// When the UPower DBus proxy is unavailable (proxy_ == nullptr) all queries
// return unknown/negative defaults and refresh() is a no-op.
//
// Thread safety: not thread-safe; call from a single thread or guard
// externally.
class linux_battery final : public battery {
public:
    linux_battery();

    linux_battery(const linux_battery&)            = delete;
    linux_battery& operator=(const linux_battery&) = delete;
    linux_battery(linux_battery&&)                 = delete;
    linux_battery& operator=(linux_battery&&)      = delete;

    ~linux_battery() override;

    // Returns the current charge level in [0.0, 1.0], or -1.0 if unknown.
    [[nodiscard]] double               charge_level()  const override;
    [[nodiscard]] battery_state        state()         const override;
    [[nodiscard]] battery_power_source power_source()  const override;

    // Energy-saver is not exposed by UPower; always returns unknown.
    [[nodiscard]] energy_saver_status  energy_saver()  const override;

    // Re-reads all properties from the UPower proxy and emits change signals.
    void refresh();

private:
    // Opaque GDBusProxy* stored as void* to keep GIO out of the public header.
    void* proxy_ = nullptr;

    // Cached snapshot — populated in ctor and updated by refresh().
    battery_info cached_info_{};

    // Reads all properties from proxy_ and returns a fresh battery_info.
    // Returns a default (unknown) snapshot when proxy_ is null.
    [[nodiscard]] battery_info read_info() const noexcept;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_LINUX_BATTERY_LINUX_HPP
