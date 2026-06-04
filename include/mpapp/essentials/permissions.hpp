// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::permissions` — runtime permission checking and requesting.
// Counterpart to MAUI Essentials `Permissions`. Abstract interface + an
// in-memory mock whose state is fully test-settable. Real per-platform
// backends (Windows AppCapabilities, Linux D-Bus portals, Android
// ActivityCompat, iOS AVFoundation/CLLocationManager/etc.) implement the
// same interface and are injected via the DI container (RFC-0011).
// No macros; header-only.

#ifndef MPAPP_ESSENTIALS_PERMISSIONS_HPP
#define MPAPP_ESSENTIALS_PERMISSIONS_HPP

#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace mpapp {

// Mirrors MAUI PermissionStatus.
enum class permission_status : std::uint8_t {
    unknown    = 0,  // not yet checked
    denied     = 1,  // user explicitly denied
    disabled   = 2,  // permission disabled at system level
    granted    = 3,  // permission granted
    restricted = 4,  // restricted by parental controls / MDM
    limited    = 5,  // partial grant (iOS Photos, e.g.)
};

// The set of permissions the API can check or request — mirrors MAUI
// BasePermission subtypes plus commonly needed Android/iOS permissions.
enum class permission_type : std::uint8_t {
    location_when_in_use = 0,
    location_always      = 1,
    camera               = 2,
    microphone           = 3,
    photos               = 4,
    contacts             = 5,
    calendar             = 6,
    reminders            = 7,
    sensors              = 8,
    storage_read         = 9,
    storage_write        = 10,
    phone                = 11,
    sms                  = 12,
    bluetooth            = 13,
    network_state        = 14,
};

[[nodiscard]] constexpr std::string_view to_string(permission_status s) noexcept {
    switch (s) {
        case permission_status::unknown:    return "unknown";
        case permission_status::denied:     return "denied";
        case permission_status::disabled:   return "disabled";
        case permission_status::granted:    return "granted";
        case permission_status::restricted: return "restricted";
        case permission_status::limited:    return "limited";
        default:                            return "?";
    }
}

[[nodiscard]] constexpr std::string_view to_string(permission_type t) noexcept {
    switch (t) {
        case permission_type::location_when_in_use: return "location_when_in_use";
        case permission_type::location_always:      return "location_always";
        case permission_type::camera:               return "camera";
        case permission_type::microphone:           return "microphone";
        case permission_type::photos:               return "photos";
        case permission_type::contacts:             return "contacts";
        case permission_type::calendar:             return "calendar";
        case permission_type::reminders:            return "reminders";
        case permission_type::sensors:              return "sensors";
        case permission_type::storage_read:         return "storage_read";
        case permission_type::storage_write:        return "storage_write";
        case permission_type::phone:                return "phone";
        case permission_type::sms:                  return "sms";
        case permission_type::bluetooth:            return "bluetooth";
        case permission_type::network_state:        return "network_state";
        default:                                    return "?";
    }
}

// Abstract interface — mirrors MAUI Permissions semantics.
class permissions {
public:
    virtual ~permissions() = default;

    // Returns the current grant status without prompting the user.
    [[nodiscard]] virtual permission_status check_status(permission_type type) const = 0;

    // Requests the permission from the user. Returns the resulting status.
    // On platforms where requesting is not meaningful (e.g. Windows, Linux
    // desktop) this typically returns `granted` or the current status.
    virtual permission_status request(permission_type type) = 0;

    // True when the app should show a rationale UI before requesting the
    // permission again (Android only; returns false on other platforms).
    [[nodiscard]] virtual bool should_show_rationale(permission_type type) const = 0;
};

// Mock / in-memory implementation.
//
// * `set_status(type, status)` — preset the check_status() return value.
// * `set_request_result(type, status)` — preset the request() return value
//   independently from check_status(). If no request result is set, request()
//   falls back to the current check_status() value.
// * `set_rationale(type, show)` — control should_show_rationale().
// * `requested_types()` — returns every permission_type passed to request()
//   in call order so tests can assert the sequence.
// * `last_requested()` — returns the most-recently requested type, or
//   std::nullopt if request() has never been called.
class mock_permissions final : public permissions {
public:
    mock_permissions() = default;

    // ---- permissions interface --------------------------------------------

    [[nodiscard]] permission_status check_status(permission_type type) const override {
        auto it = statuses_.find(static_cast<std::uint8_t>(type));
        if (it == statuses_.end()) {
            return permission_status::unknown;
        }
        return it->second;
    }

    permission_status request(permission_type type) override {
        requested_types_.push_back(type);
        last_requested_ = type;

        auto it = request_results_.find(static_cast<std::uint8_t>(type));
        if (it != request_results_.end()) {
            // Promote check_status to the granted result so subsequent
            // check_status() calls reflect the request outcome.
            statuses_[static_cast<std::uint8_t>(type)] = it->second;
            return it->second;
        }
        return check_status(type);
    }

    [[nodiscard]] bool should_show_rationale(permission_type type) const override {
        auto it = rationale_.find(static_cast<std::uint8_t>(type));
        return it != rationale_.end() && it->second;
    }

    // ---- Test-control helpers --------------------------------------------

    // Preset the value returned by check_status() for a given type.
    void set_status(permission_type type, permission_status status) {
        statuses_[static_cast<std::uint8_t>(type)] = status;
    }

    // Preset the value returned (and written back to check_status) by
    // request() for a given type. If unset, request() echoes check_status().
    void set_request_result(permission_type type, permission_status result) {
        request_results_[static_cast<std::uint8_t>(type)] = result;
    }

    // Control whether should_show_rationale() returns true for a type.
    void set_rationale(permission_type type, bool show) {
        rationale_[static_cast<std::uint8_t>(type)] = show;
    }

    // Ordered list of every type passed to request().
    [[nodiscard]] const std::vector<permission_type>& requested_types() const noexcept {
        return requested_types_;
    }

    // Most-recently requested type, or nullopt if request() was never called.
    [[nodiscard]] std::optional<permission_type> last_requested() const noexcept {
        return last_requested_;
    }

    // Reset all preset state and call history.
    void reset() {
        statuses_.clear();
        request_results_.clear();
        rationale_.clear();
        requested_types_.clear();
        last_requested_ = std::nullopt;
    }

private:
    std::unordered_map<std::uint8_t, permission_status> statuses_{};
    std::unordered_map<std::uint8_t, permission_status> request_results_{};
    std::unordered_map<std::uint8_t, bool>              rationale_{};
    std::vector<permission_type>                        requested_types_{};
    std::optional<permission_type>                      last_requested_{};
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_PERMISSIONS_HPP
