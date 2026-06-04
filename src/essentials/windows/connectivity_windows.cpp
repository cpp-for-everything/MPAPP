// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Win32 implementation of `mpapp::windows_connectivity`.
// windows.h and wininet.h are confined to this translation unit.

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wininet.h>

#include "mpapp/essentials/windows/connectivity_windows.hpp"

namespace {

// Query WinINet for a best-effort connected state.
// Returns network_access::internet if the machine has internet-level
// connectivity according to InternetGetConnectedState, otherwise
// network_access::none.
//
// InternetGetConnectedState does not distinguish local-only or captive-
// portal (constrained) scenarios.  Mapping those states requires the
// NLM/INetworkListManager COM interface — deferred to a follow-up.
[[nodiscard]] mpapp::network_access query_access() noexcept
{
    DWORD flags = 0;
    if (::InternetGetConnectedState(&flags, 0) != FALSE) {
        return mpapp::network_access::internet;
    }
    return mpapp::network_access::none;
}

} // anonymous namespace

namespace mpapp {

windows_connectivity::windows_connectivity()
    : access_{ query_access() }
{}

network_access windows_connectivity::access() const
{
    return access_;
}

void windows_connectivity::refresh()
{
    const network_access current = query_access();
    if (current == access_) {
        return;
    }
    access_ = current;
    connectivity_changed.emit(current);
}

} // namespace mpapp

#endif // defined(_WIN32)
