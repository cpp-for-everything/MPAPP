// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::geocoding` — forward and reverse geocoding. Counterpart to MAUI
// Essentials `Geocoding`. Abstract interface + an in-memory mock whose
// registered address/coordinate tables are settable so tests can drive
// both `get_locations_for_address` and `get_placemarks` deterministically.
// Real per-platform backends (Windows MapLocationFinder, Linux Nominatim,
// Android Geocoder, iOS CLGeocoder) implement the same interface and are
// injected via the DI container (RFC-0011). No macros; header-only
// interface. Self-contained: does NOT depend on geolocation.hpp.

#ifndef MPAPP_ESSENTIALS_GEOCODING_HPP
#define MPAPP_ESSENTIALS_GEOCODING_HPP

#include <string>
#include <unordered_map>
#include <vector>

namespace mpapp {

// A geographic place description. Mirrors MAUI Essentials `Placemark`.
// All string fields default to empty; numeric fields default to 0.
struct placemark {
    std::string country_name{};
    std::string country_code{};
    std::string admin_area{};
    std::string sub_admin_area{};
    std::string locality{};
    std::string sub_locality{};
    std::string thoroughfare{};
    std::string postal_code{};
    std::string feature_name{};

    double latitude  = 0.0;
    double longitude = 0.0;

    bool operator==(const placemark&) const = default;
};

// Abstract geocoding interface.
class geocoding {
public:
    virtual ~geocoding() = default;

    // Forward geocoding: resolve a human-readable address string to one or
    // more placemarks. Returns an empty vector when no results are found or
    // the service is unsupported.
    [[nodiscard]] virtual std::vector<placemark>
        get_locations_for_address(const std::string& address) const = 0;

    // Reverse geocoding: resolve a latitude/longitude pair to one or more
    // placemarks. Returns an empty vector when no results are found or the
    // service is unsupported.
    [[nodiscard]] virtual std::vector<placemark>
        get_placemarks(double latitude, double longitude) const = 0;
};

// Mock / in-memory implementation.
//
// * register_address(addr, placemarks) — pre-load results for forward
//   geocoding. Exact string key match.
// * register_coordinates(lat, lng, placemarks) — pre-load results for
//   reverse geocoding. The key is a stringified "lat,lng" pair with full
//   double precision so callers must use the exact same values they register.
// * get_locations_for_address() returns the registered vector, or empty.
// * get_placemarks() returns the registered vector, or empty.
// * last_address_query() / last_latitude_query() / last_longitude_query()
//   return the arguments of the most recent call to each method (useful for
//   asserting which query was issued).
class mock_geocoding final : public geocoding {
public:
    // ---- geocoding interface -------------------------------------------------

    [[nodiscard]] std::vector<placemark>
    get_locations_for_address(const std::string& address) const override {
        last_address_query_ = address;
        auto it = address_table_.find(address);
        if (it == address_table_.end()) {
            return {};
        }
        return it->second;
    }

    [[nodiscard]] std::vector<placemark>
    get_placemarks(double latitude, double longitude) const override {
        last_lat_query_ = latitude;
        last_lng_query_ = longitude;
        auto key = make_coord_key(latitude, longitude);
        auto it  = coord_table_.find(key);
        if (it == coord_table_.end()) {
            return {};
        }
        return it->second;
    }

    // ---- Test-control helpers -----------------------------------------------

    // Pre-load forward-geocoding results for `address`.
    void register_address(const std::string& address,
                          std::vector<placemark> results) {
        address_table_[address] = std::move(results);
    }

    // Pre-load reverse-geocoding results for the (lat, lng) pair.
    void register_coordinates(double latitude, double longitude,
                              std::vector<placemark> results) {
        coord_table_[make_coord_key(latitude, longitude)] = std::move(results);
    }

    // The address string passed to the most recent get_locations_for_address().
    // Empty string if never called.
    [[nodiscard]] const std::string& last_address_query() const noexcept {
        return last_address_query_;
    }

    // The latitude passed to the most recent get_placemarks(). 0.0 if never
    // called.
    [[nodiscard]] double last_latitude_query() const noexcept {
        return last_lat_query_;
    }

    // The longitude passed to the most recent get_placemarks(). 0.0 if never
    // called.
    [[nodiscard]] double last_longitude_query() const noexcept {
        return last_lng_query_;
    }

private:
    // Build a deterministic map key from a coordinate pair. Uses the full
    // bit-for-bit representation of each double so that callers who register
    // and query with the exact same literal values always match.
    [[nodiscard]] static std::string make_coord_key(double lat, double lng) {
        return std::to_string(lat) + "," + std::to_string(lng);
    }

    std::unordered_map<std::string, std::vector<placemark>> address_table_{};
    std::unordered_map<std::string, std::vector<placemark>> coord_table_{};

    mutable std::string last_address_query_{};
    mutable double      last_lat_query_ = 0.0;
    mutable double      last_lng_query_ = 0.0;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_GEOCODING_HPP
