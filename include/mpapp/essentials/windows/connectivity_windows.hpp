// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::windows_connectivity` — Win32 network reachability backend.
// Implements `mpapp::connectivity` using InternetGetConnectedState (WinINet).
// No windows.h in this header; all Win32/WinINet details are confined to
// the .cpp translation unit.
//
// NOTE: The best-effort InternetGetConnectedState API maps to
// network_access::internet (connected) or network_access::none (not
// connected) only — the local/constrained distinctions are not available
// without NLM/INetworkListManager COM, which is deferred to a follow-up.

#ifndef MPAPP_ESSENTIALS_WINDOWS_CONNECTIVITY_WINDOWS_HPP
#define MPAPP_ESSENTIALS_WINDOWS_CONNECTIVITY_WINDOWS_HPP

#include "../../essentials/connectivity.hpp"

namespace mpapp {

// Win32 connectivity backend. Uses InternetGetConnectedState (WinINet)
// to determine whether the machine has internet access.
//
// access() returns:
//   - network_access::internet  when WinINet reports a connection
//   - network_access::none      otherwise
//
// refresh() re-reads the state; if it has changed it updates the cached
// value and emits connectivity_changed.
//
// TODO (follow-up): replace the best-effort WinINet probe with the full
// INetworkListManager COM interface (NLM) for local/constrained/internet
// distinction and callback-driven change notifications.
class windows_connectivity final : public connectivity {
public:
    windows_connectivity();
    ~windows_connectivity() = default;

    windows_connectivity(const windows_connectivity&)            = delete;
    windows_connectivity& operator=(const windows_connectivity&) = delete;
    windows_connectivity(windows_connectivity&&)                 = delete;
    windows_connectivity& operator=(windows_connectivity&&)      = delete;

    // Returns the last-cached network access level.
    [[nodiscard]] network_access access() const override;

    // Re-reads the current state via InternetGetConnectedState.
    // Emits connectivity_changed if the access level has changed since the
    // last call (or since construction).
    void refresh();

private:
    network_access access_;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_WINDOWS_CONNECTIVITY_WINDOWS_HPP
