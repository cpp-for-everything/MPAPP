// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::android_geolocation` — Android LocationManager backend.
// Implements `mpapp::geolocation` using android.location.LocationManager,
// reached through the app Context obtained from the JNI bridge
// (mpapp::detail::get_activity()). All JNI details (<jni.h>, FindClass,
// CallObjectMethod, DeleteLocalRef, AttachCurrentThread) are confined to
// the .cpp translation unit; this header stays JNI-free, mirroring every
// other Android backend in the project. No macros in the public API.
//
// Synchronous path (get_last_known / get_location):
//   LocationManager.getLastKnownLocation(GPS_PROVIDER | NETWORK_PROVIDER)
//   returns a cached android.location.Location. If GPS returns null the
//   implementation falls back to NETWORK_PROVIDER.
//
// Continuous path (start_listening / stop_listening):
//   Calling start_listening() sets the is_listening flag. A real
//   LocationManager.requestLocationUpdates() invocation requires an Android
//   Looper on the calling thread (or a Handler) and a LocationListener
//   callback object registered in the JVM. That wiring is a follow-up task;
//   the state bookkeeping (listening_/request_) is complete here and the
//   stub can be replaced in-place without changing the interface.
//
// Thread safety: all public methods marshal onto a JNIEnv obtained via
// detail::attach_current_thread(); the underlying AndroidVM handles
// simultaneous attachment from multiple threads.

#ifndef MPAPP_ESSENTIALS_ANDROID_GEOLOCATION_ANDROID_HPP
#define MPAPP_ESSENTIALS_ANDROID_GEOLOCATION_ANDROID_HPP

#include <optional>
#include <string>

#include "../../essentials/geolocation.hpp"

namespace mpapp {

// Android geolocation backend. Implements `mpapp::geolocation` via
// android.location.LocationManager obtained from the app Context with
// Context.getSystemService(Context.LOCATION_SERVICE).
//
// The Context is taken from the JNI bridge (detail::get_activity()), set
// once during native init by the host MainActivity — no Context is required
// in the constructor.
//
// get_last_known()  -> LocationManager.getLastKnownLocation(GPS_PROVIDER),
//                      fallback to NETWORK_PROVIDER; fills geo_location from
//                      the returned android.location.Location object.
// get_location()    -> same query path; stores the request parameters.
// is_listening()    -> returns the current listening state flag.
// start_listening() -> sets listening flag; full requestLocationUpdates
//                      wiring (needs a Looper + LocationListener JVM object)
//                      is a follow-up task.
// stop_listening()  -> clears the listening flag; removeUpdates follow-up.
class android_geolocation final : public geolocation {
public:
    android_geolocation()  = default;
    ~android_geolocation() = default;

    android_geolocation(const android_geolocation&)            = delete;
    android_geolocation& operator=(const android_geolocation&) = delete;
    android_geolocation(android_geolocation&&)                 = delete;
    android_geolocation& operator=(android_geolocation&&)      = delete;

    // Returns the most recent cached fix from LocationManager, or
    // std::nullopt if unavailable (no permission, no provider, JNI error).
    [[nodiscard]] std::optional<geo_location> get_last_known() const override;

    // Performs a synchronous getLastKnownLocation query with the supplied
    // accuracy hint. Falls back from GPS_PROVIDER to NETWORK_PROVIDER.
    // Returns std::nullopt when no fix is available.
    [[nodiscard]] std::optional<geo_location>
        get_location(const geolocation_request& req = {}) override;

    // Returns true while continuous listening is active.
    [[nodiscard]] bool is_listening() const override;

    // Marks continuous listening as active and stores the request parameters.
    // Calling start_listening() while already listening is a no-op.
    // Full requestLocationUpdates wiring is a follow-up task.
    void start_listening(const geolocation_request& req = {}) override;

    // Marks continuous listening as inactive. Calling stop_listening() while
    // not listening is a no-op.
    void stop_listening() override;

private:
    bool                listening_    = false;
    geolocation_request last_request_ {};
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_ANDROID_GEOLOCATION_ANDROID_HPP
