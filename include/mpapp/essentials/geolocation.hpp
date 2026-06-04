// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::geolocation` — device location services. Counterpart to MAUI
// Essentials `Geolocation`. Abstract interface + an in-memory mock whose
// state is settable so tests can drive `location_changed`. Supports both
// one-shot location queries and continuous listening. Real per-platform
// backends (Windows LocationService, Linux GeoClue, Android
// FusedLocationProviderClient, iOS CLLocationManager) implement the same
// interface and are injected via the DI container (RFC-0011). No macros;
// header-only interface.

#ifndef MPAPP_ESSENTIALS_GEOLOCATION_HPP
#define MPAPP_ESSENTIALS_GEOLOCATION_HPP

#include <cstdint>
#include <optional>
#include <string_view>

#include "../signal.hpp"

namespace mpapp {

// Desired accuracy for a location request. Mirrors MAUI GeolocationAccuracy.
enum class geolocation_accuracy : std::uint8_t {
    lowest = 0,  // ~3 km
    low    = 1,  // ~1 km
    medium = 2,  // ~100 m
    high   = 3,  // ~10 m
    best   = 4,  // maximum precision
};

[[nodiscard]] constexpr std::string_view to_string(geolocation_accuracy a) noexcept {
    switch (a) {
        case geolocation_accuracy::lowest: return "lowest";
        case geolocation_accuracy::low:    return "low";
        case geolocation_accuracy::medium: return "medium";
        case geolocation_accuracy::high:   return "high";
        case geolocation_accuracy::best:   return "best";
    }
    return "unknown";
}

// A geographic fix. All fields mirror MAUI Location properties.
struct geo_location {
    double latitude  = 0.0;
    double longitude = 0.0;

    std::optional<double> altitude{};   // metres above sea level
    std::optional<double> accuracy{};   // horizontal accuracy radius (metres)
    std::optional<double> speed{};      // metres per second
    std::optional<double> course{};     // degrees clockwise from true north

    // Unix epoch milliseconds; 0 means unset.
    long long timestamp = 0;

    bool operator==(const geo_location&) const = default;
};

// Parameters passed to get_location() and start_listening().
struct geolocation_request {
    geolocation_accuracy accuracy       = geolocation_accuracy::medium;
    double               timeout_seconds = 0.0;  // 0 = no explicit timeout
};

// Abstract geolocation interface.
class geolocation {
public:
    virtual ~geolocation() = default;

    // Returns the most recent cached fix, or std::nullopt if unavailable.
    [[nodiscard]] virtual std::optional<geo_location> get_last_known() const = 0;

    // Perform a one-shot location query with the supplied request parameters.
    // Returns the fix, or std::nullopt when the device cannot obtain one.
    [[nodiscard]] virtual std::optional<geo_location>
        get_location(const geolocation_request& req = {}) = 0;

    // Whether continuous listening is currently active.
    [[nodiscard]] virtual bool is_listening() const = 0;

    // Begin emitting location_changed signals whenever the position updates.
    // Calling start_listening() while already listening is a no-op.
    virtual void start_listening(const geolocation_request& req = {}) = 0;

    // Stop emitting location_changed signals.
    // Calling stop_listening() while not listening is a no-op.
    virtual void stop_listening() = 0;

    // Fires with each new fix while continuous listening is active.
    mpapp::signal<geo_location> location_changed{};
};

// Mock / in-memory implementation.
//
// * set_last_known(loc) / set_current(loc) configure what the two query
//   methods return.
// * start_listening() / stop_listening() toggle is_listening().
// * push_location(loc) emits location_changed only while listening; the call
//   is silently ignored otherwise. It also updates the last_known cache.
// * last_request() returns the geolocation_request passed to the most
//   recent start_listening() or get_location() call.
class mock_geolocation final : public geolocation {
public:
    // ---- geolocation interface -----------------------------------------------

    [[nodiscard]] std::optional<geo_location> get_last_known() const override {
        return last_known_;
    }

    [[nodiscard]] std::optional<geo_location>
    get_location(const geolocation_request& req = {}) override {
        last_request_ = req;
        return current_;
    }

    [[nodiscard]] bool is_listening() const override { return listening_; }

    void start_listening(const geolocation_request& req = {}) override {
        if (listening_) {
            return;
        }
        last_request_ = req;
        listening_    = true;
    }

    void stop_listening() override {
        listening_ = false;
    }

    // ---- Test-control helpers ------------------------------------------------

    // Inject the value returned by get_last_known().
    void set_last_known(std::optional<geo_location> loc) {
        last_known_ = std::move(loc);
    }

    // Inject the value returned by get_location().
    void set_current(std::optional<geo_location> loc) {
        current_ = std::move(loc);
    }

    // Push a new fix: emits location_changed only while listening; also
    // updates the last_known cache regardless of listening state.
    void push_location(geo_location loc) {
        last_known_ = loc;
        if (!listening_) {
            return;
        }
        location_changed.emit(loc);
    }

    // The request parameters supplied to the most recent start_listening() or
    // get_location() call.
    [[nodiscard]] const geolocation_request& last_request() const noexcept {
        return last_request_;
    }

private:
    std::optional<geo_location> last_known_{};
    std::optional<geo_location> current_{};
    bool                        listening_    = false;
    geolocation_request         last_request_{};
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_GEOLOCATION_HPP
