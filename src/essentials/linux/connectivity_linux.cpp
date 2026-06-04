// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Linux GIO implementation of `mpapp::linux_connectivity`.
// <gio/gio.h> is confined to this translation unit.

#include "mpapp/essentials/linux/connectivity_linux.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gio/gio.h>

namespace {

// Translate GNetworkConnectivity to mpapp::network_access.
[[nodiscard]] mpapp::network_access
translate_connectivity(GNetworkConnectivity level) noexcept
{
    switch (level) {
        case G_NETWORK_CONNECTIVITY_FULL:
            return mpapp::network_access::internet;
        case G_NETWORK_CONNECTIVITY_LOCAL:
            return mpapp::network_access::local;
        case G_NETWORK_CONNECTIVITY_LIMITED:
            return mpapp::network_access::constrained;
        default:
            return mpapp::network_access::none;
    }
}

// Read the current access level from a live GNetworkMonitor.
[[nodiscard]] mpapp::network_access
read_access(GNetworkMonitor* monitor) noexcept
{
    if (monitor == nullptr) {
        return mpapp::network_access::none;
    }
    // If the monitor says no network is available at all, report none
    // regardless of the connectivity enum value.
    if (!g_network_monitor_get_network_available(monitor)) {
        return mpapp::network_access::none;
    }
    const GNetworkConnectivity level =
        g_network_monitor_get_connectivity(monitor);
    return translate_connectivity(level);
}

} // anonymous namespace

namespace mpapp {

linux_connectivity::linux_connectivity()
{
    GNetworkMonitor* mon = g_network_monitor_get_default();
    if (mon == nullptr) {
        return;
    }

    monitor_ = static_cast<void*>(mon);
    current_access_ = read_access(mon);

    // Connect the "network-changed" signal.
    // Signature: void callback(GNetworkMonitor*, gboolean available, gpointer)
    signal_handler_ = static_cast<std::uint64_t>(
        g_signal_connect(mon,
                         "network-changed",
                         G_CALLBACK(on_network_changed),
                         this));
}

linux_connectivity::~linux_connectivity()
{
    if (monitor_ != nullptr && signal_handler_ != 0) {
        g_signal_handler_disconnect(
            static_cast<GNetworkMonitor*>(monitor_),
            static_cast<gulong>(signal_handler_));
    }
    // GNetworkMonitor is a singleton owned by GIO; do NOT g_object_unref it.
    monitor_        = nullptr;
    signal_handler_ = 0;
}

network_access linux_connectivity::access() const
{
    return current_access_;
}

// static
void linux_connectivity::on_network_changed(void* /*monitor*/,
                                             int   /*available*/,
                                             void* user_data) noexcept
{
    auto* self = static_cast<linux_connectivity*>(user_data);
    if (self == nullptr) {
        return;
    }
    self->update_access();
}

void linux_connectivity::update_access() noexcept
{
    if (monitor_ == nullptr) {
        return;
    }
    const network_access next =
        read_access(static_cast<GNetworkMonitor*>(monitor_));
    if (next == current_access_) {
        return;
    }
    current_access_ = next;
    connectivity_changed.emit(next);
}

} // namespace mpapp

#endif // defined(__linux__) && !defined(__ANDROID__)
