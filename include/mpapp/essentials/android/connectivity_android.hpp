// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::android_connectivity` — Android network reachability backend.
// Implements `mpapp::connectivity` using android.net.ConnectivityManager,
// reached through the app Context obtained from the JNI bridge
// (mpapp::detail::get_activity()). All JNI details (<jni.h>, FindClass,
// CallObjectMethod, DeleteLocalRef, AttachCurrentThread) are confined to
// the .cpp translation unit; this header stays JNI-free, mirroring the
// clipboard and vibration backends. No macros in the public API.

#ifndef MPAPP_ESSENTIALS_ANDROID_CONNECTIVITY_ANDROID_HPP
#define MPAPP_ESSENTIALS_ANDROID_CONNECTIVITY_ANDROID_HPP

#include "mpapp/essentials/connectivity.hpp"

namespace mpapp {

// Android connectivity backend. Implements `mpapp::connectivity` via
// android.net.ConnectivityManager, obtained from the app Context with
// Context.getSystemService(Context.CONNECTIVITY_SERVICE).
//
// The Context is taken from the JNI bridge (detail::get_activity()), which
// the host MainActivity sets once during native init.
//
// access() queries ConnectivityManager.getActiveNetwork() to obtain the
// current Network, then calls getNetworkCapabilities(Network) to test:
//   NET_CAPABILITY_INTERNET + NET_CAPABILITY_VALIDATED -> network_access::internet
//   NET_CAPABILITY_INTERNET only (not yet validated)   -> network_access::local
//   Active network but neither capability               -> network_access::local
//   No active network (getActiveNetwork() returns null) -> network_access::none
//
// This is a synchronous point-in-time query; call access() from any thread.
// ConnectivityManager change callbacks are not wired in this class; callers
// that need live change notifications should combine this class with a
// platform change listener that calls connectivity_changed.emit() directly.
class android_connectivity final : public connectivity {
public:
    android_connectivity()  = default;
    ~android_connectivity() = default;

    android_connectivity(const android_connectivity&)            = delete;
    android_connectivity& operator=(const android_connectivity&) = delete;
    android_connectivity(android_connectivity&&)                 = delete;
    android_connectivity& operator=(android_connectivity&&)      = delete;

    // Query the current network reachability via ConnectivityManager.
    // Returns network_access::none  when there is no active network,
    //         network_access::local when there is a network but internet
    //                               access is not validated,
    //         network_access::internet when NET_CAPABILITY_INTERNET +
    //                               NET_CAPABILITY_VALIDATED are both set.
    // Falls back to network_access::none when the bridge or Context is
    // unavailable (e.g. called before set_activity()).
    [[nodiscard]] network_access access() const override;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_ANDROID_CONNECTIVITY_ANDROID_HPP
