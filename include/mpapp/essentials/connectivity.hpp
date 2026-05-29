// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::connectivity` — network reachability + a change signal.
// Counterpart to MAUI Essentials `Connectivity`. Interface + a mock
// whose state is settable so tests can drive `connectivity_changed`.
// Real backends (Windows NetworkInformation, Linux NetworkManager,
// Android ConnectivityManager) implement the same interface. No macros.

#ifndef MPAPP_ESSENTIALS_CONNECTIVITY_HPP
#define MPAPP_ESSENTIALS_CONNECTIVITY_HPP

#include <cstdint>

#include "../signal.hpp"

namespace mpapp {

// MAUI's NetworkAccess.
enum class network_access : std::uint8_t {
    none        = 0,  // no connectivity
    local       = 1,  // local network only
    constrained = 2,  // captive-portal / limited
    internet    = 3,  // full internet
};

class connectivity {
public:
    virtual ~connectivity() = default;

    [[nodiscard]] virtual network_access access() const = 0;
    [[nodiscard]] bool is_online() const { return access() == network_access::internet; }

    // Fires whenever the access level changes.
    mpapp::signal<network_access> connectivity_changed{};
};

// Mock implementation: state is settable; setting a NEW value fires the
// change signal.
class mock_connectivity final : public connectivity {
public:
    explicit mock_connectivity(network_access initial = network_access::internet)
        : access_{ initial } {}

    [[nodiscard]] network_access access() const override { return access_; }

    void set_access(network_access a) {
        if (a == access_) {
            return;
        }
        access_ = a;
        connectivity_changed.emit(a);
    }

private:
    network_access access_;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_CONNECTIVITY_HPP
