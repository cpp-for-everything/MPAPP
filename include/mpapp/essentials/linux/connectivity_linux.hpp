// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::linux_connectivity` — GNetworkMonitor backend for Linux.
// Implements `mpapp::connectivity` using GNetworkMonitor from GIO.
// All GIO/GLib headers are confined to the .cpp translation unit;
// this header exposes only the class declaration.

#ifndef MPAPP_ESSENTIALS_LINUX_CONNECTIVITY_LINUX_HPP
#define MPAPP_ESSENTIALS_LINUX_CONNECTIVITY_LINUX_HPP

#include <cstdint>

#include "../../essentials/connectivity.hpp"

namespace mpapp {

// GIO GNetworkMonitor backend. Implements `mpapp::connectivity` via
// g_network_monitor_get_default().
//
// access() maps GNetworkConnectivity levels to network_access:
//   G_NETWORK_CONNECTIVITY_FULL    -> network_access::internet
//   G_NETWORK_CONNECTIVITY_LOCAL   -> network_access::local
//   G_NETWORK_CONNECTIVITY_LIMITED -> network_access::constrained
//   (any other / unavailable)      -> network_access::none
//
// Connects the GNetworkMonitor::network-changed GLib signal in the ctor
// and disconnects in the dtor. Emits connectivity_changed whenever the
// access level transitions to a new value.
//
// When GNetworkMonitor is unavailable (g_network_monitor_get_default()
// returns null) all operations degrade gracefully: access() returns
// network_access::none, no signal is connected.
//
// Thread safety: not thread-safe; call from a single thread or guard
// externally.
class linux_connectivity final : public connectivity {
public:
    linux_connectivity();

    linux_connectivity(const linux_connectivity&)            = delete;
    linux_connectivity& operator=(const linux_connectivity&) = delete;
    linux_connectivity(linux_connectivity&&)                 = delete;
    linux_connectivity& operator=(linux_connectivity&&)      = delete;

    ~linux_connectivity() override;

    // Returns the current network access level derived from GNetworkMonitor.
    [[nodiscard]] network_access access() const override;

private:
    // Opaque GNetworkMonitor pointer (typed as void* to keep GIO out of the
    // public header; the .cpp casts it back to GNetworkMonitor*).
    void*         monitor_         = nullptr;
    std::uint64_t signal_handler_  = 0;

    // Last cached access level, updated on every network-changed callback.
    network_access current_access_ = network_access::none;

    // GLib static trampoline: user_data is the linux_connectivity instance.
    // Signature matches GNetworkMonitor::network-changed (gboolean available).
    static void on_network_changed(void* monitor,
                                   int   available,
                                   void* user_data) noexcept;

    // Re-reads GNetworkMonitor connectivity and updates current_access_.
    // Emits connectivity_changed when the level differs from the cached value.
    void update_access() noexcept;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_LINUX_CONNECTIVITY_LINUX_HPP
