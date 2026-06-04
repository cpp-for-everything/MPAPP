// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Linux GDBus/UPower implementation of `mpapp::linux_battery`.
// All GIO/GDBus headers are confined to this translation unit.

#include "mpapp/essentials/linux/battery_linux.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gio/gio.h>

namespace {

// UPower D-Bus service / object / interface constants.
constexpr const char* k_upower_service    = "org.freedesktop.UPower";
constexpr const char* k_upower_object     = "/org/freedesktop/UPower/devices/DisplayDevice";
constexpr const char* k_upower_iface      = "org.freedesktop.UPower.Device";
constexpr const char* k_upower_root_iface = "org.freedesktop.UPower";
constexpr const char* k_upower_root_obj   = "/org/freedesktop/UPower";

// UPower State enum values (uint32).
constexpr guint32 k_state_charging           = 1;
constexpr guint32 k_state_discharging        = 2;
constexpr guint32 k_state_empty              = 3;
constexpr guint32 k_state_full               = 4;
constexpr guint32 k_state_pending_charge     = 5;
constexpr guint32 k_state_pending_discharge  = 6;

// Extract a double property from a GDBusProxy. Returns fallback on error.
[[nodiscard]] double
proxy_get_double(GDBusProxy* proxy, const char* property, double fallback) noexcept
{
    if (proxy == nullptr) { return fallback; }
    GVariant* var = g_dbus_proxy_get_cached_property(proxy, property);
    if (var == nullptr) { return fallback; }
    const double value = g_variant_get_double(var);
    g_variant_unref(var);
    return value;
}

// Extract a uint32 property from a GDBusProxy. Returns fallback on error.
[[nodiscard]] guint32
proxy_get_uint32(GDBusProxy* proxy, const char* property, guint32 fallback) noexcept
{
    if (proxy == nullptr) { return fallback; }
    GVariant* var = g_dbus_proxy_get_cached_property(proxy, property);
    if (var == nullptr) { return fallback; }
    const guint32 value = g_variant_get_uint32(var);
    g_variant_unref(var);
    return value;
}

// Extract a boolean property from a GDBusProxy. Returns fallback on error.
[[nodiscard]] gboolean
proxy_get_bool(GDBusProxy* proxy, const char* property, gboolean fallback) noexcept
{
    if (proxy == nullptr) { return fallback; }
    GVariant* var = g_dbus_proxy_get_cached_property(proxy, property);
    if (var == nullptr) { return fallback; }
    const gboolean value = g_variant_get_boolean(var);
    g_variant_unref(var);
    return value;
}

// Map UPower State uint32 to mpapp::battery_state.
[[nodiscard]] mpapp::battery_state
translate_state(guint32 upower_state) noexcept
{
    switch (upower_state) {
        case k_state_charging:
        case k_state_pending_charge:
            return mpapp::battery_state::charging;
        case k_state_discharging:
        case k_state_pending_discharge:
            return mpapp::battery_state::discharging;
        case k_state_full:
            return mpapp::battery_state::full;
        case k_state_empty:
            return mpapp::battery_state::not_present;
        default:
            return mpapp::battery_state::unknown;
    }
}

// Read OnBattery from the root UPower proxy to determine power source.
// Returns battery_power_source::unknown if proxy is null.
[[nodiscard]] mpapp::battery_power_source
read_power_source_from_root() noexcept
{
    GError* err = nullptr;
    GDBusProxy* root_proxy = g_dbus_proxy_new_for_bus_sync(
        G_BUS_TYPE_SYSTEM,
        G_DBUS_PROXY_FLAGS_NONE,
        nullptr,
        k_upower_service,
        k_upower_root_obj,
        k_upower_root_iface,
        nullptr,
        &err);

    if (err != nullptr) {
        g_error_free(err);
    }
    if (root_proxy == nullptr) {
        return mpapp::battery_power_source::unknown;
    }

    const gboolean on_battery = proxy_get_bool(root_proxy, "OnBattery", FALSE);
    g_object_unref(root_proxy);

    return on_battery
        ? mpapp::battery_power_source::battery
        : mpapp::battery_power_source::ac;
}

} // anonymous namespace

namespace mpapp {

linux_battery::linux_battery()
{
    GError* err = nullptr;
    GDBusProxy* proxy = g_dbus_proxy_new_for_bus_sync(
        G_BUS_TYPE_SYSTEM,
        G_DBUS_PROXY_FLAGS_NONE,
        nullptr,
        k_upower_service,
        k_upower_object,
        k_upower_iface,
        nullptr,
        &err);

    if (err != nullptr) {
        g_error_free(err);
    }

    proxy_ = static_cast<void*>(proxy);
    cached_info_ = read_info();
}

linux_battery::~linux_battery()
{
    if (proxy_ != nullptr) {
        g_object_unref(static_cast<GDBusProxy*>(proxy_));
        proxy_ = nullptr;
    }
}

double linux_battery::charge_level() const
{
    return cached_info_.charge_level;
}

battery_state linux_battery::state() const
{
    return cached_info_.state;
}

battery_power_source linux_battery::power_source() const
{
    return cached_info_.power_source;
}

energy_saver_status linux_battery::energy_saver() const
{
    // UPower does not expose an energy-saver / low-power mode flag.
    return energy_saver_status::unknown;
}

void linux_battery::refresh()
{
    if (proxy_ == nullptr) { return; }

    const battery_info next = read_info();
    if (next == cached_info_) { return; }

    cached_info_ = next;
    battery_info_changed.emit(cached_info_);
    // energy_saver_status never changes on Linux; no emission needed.
}

battery_info linux_battery::read_info() const noexcept
{
    if (proxy_ == nullptr) {
        return battery_info{
            .charge_level  = -1.0,
            .state         = battery_state::unknown,
            .power_source  = battery_power_source::unknown,
        };
    }

    auto* proxy = static_cast<GDBusProxy*>(proxy_);

    // Percentage is in [0..100]; scale to [0..1].
    const double raw_pct = proxy_get_double(proxy, "Percentage", -100.0);
    const double level   = (raw_pct >= 0.0) ? (raw_pct / 100.0) : -1.0;

    const guint32 upower_state = proxy_get_uint32(proxy, "State", 0u);
    const battery_state bs     = translate_state(upower_state);

    const battery_power_source ps = read_power_source_from_root();

    return battery_info{
        .charge_level = level,
        .state        = bs,
        .power_source = ps,
    };
}

} // namespace mpapp

#endif // defined(__linux__) && !defined(__ANDROID__)
