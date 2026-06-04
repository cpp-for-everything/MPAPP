// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::web_authenticator` — OAuth/OpenID browser-based authentication.
// Counterpart to MAUI Essentials `WebAuthenticator`. Abstract interface +
// an in-memory mock implementation that is test-drivable (settable canned
// result; records last call arguments). Real per-platform backends launch
// the system browser or an in-app browser tab and intercept the callback
// URL; they implement the same interface and are injected via the DI
// container (RFC-0011). No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_WEB_AUTHENTICATOR_HPP
#define MPAPP_ESSENTIALS_WEB_AUTHENTICATOR_HPP

#include <map>
#include <optional>
#include <string>

namespace mpapp {

// Options passed to a single authentication attempt.
struct web_authenticator_options {
    std::string url{};           // The authorization URL to open in the browser.
    std::string callback_url{};  // The redirect URI the auth server calls back with.
    bool prefers_ephemeral = false; // Hint: prefer an ephemeral (private) browser session.
};

// The parsed result of a successful authentication attempt.
// Properties are the query/fragment parameters extracted from the callback URL
// (e.g. "access_token", "code", "id_token", etc.).
struct web_authenticator_result {
    std::map<std::string, std::string> properties{};

    // Returns the value for `key`, or std::nullopt if not present.
    [[nodiscard]] std::optional<std::string> get(const std::string& key) const {
        auto it = properties.find(key);
        if (it == properties.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    // Convenience accessor for the OAuth access_token parameter.
    [[nodiscard]] std::optional<std::string> access_token() const {
        return get("access_token");
    }
};

// Abstract interface for web-based OAuth/OIDC authentication.
class web_authenticator {
public:
    virtual ~web_authenticator() = default;

    // Initiates an authentication flow using the given options. Returns the
    // parsed result on success, or std::nullopt when the user cancelled or
    // the platform does not support browser-based auth.
    [[nodiscard]] virtual std::optional<web_authenticator_result>
        authenticate(const web_authenticator_options& options) = 0;
};

// Mock / in-memory implementation. Test-drivable:
//   - Set a canned result with set_result() (or set_not_supported() to simulate
//     a platform that cannot perform browser auth).
//   - After calling authenticate(), inspect last_options() to verify the call.
class mock_web_authenticator final : public web_authenticator {
public:
    // --- Arrange helpers ---------------------------------------------------

    // Prime the mock to return `result` on the next authenticate() call.
    void set_result(std::optional<web_authenticator_result> result) {
        canned_result_ = std::move(result);
    }

    // Convenience: prime the mock to return a result with the given properties.
    void set_result(std::map<std::string, std::string> props) {
        web_authenticator_result r;
        r.properties = std::move(props);
        canned_result_ = std::move(r);
    }

    // Prime the mock to simulate cancellation / unsupported platform
    // (authenticate returns std::nullopt).
    void set_not_supported() {
        canned_result_ = std::nullopt;
    }

    // --- Act ---------------------------------------------------------------

    [[nodiscard]] std::optional<web_authenticator_result>
    authenticate(const web_authenticator_options& options) override {
        last_options_     = options;
        call_count_      += 1;
        return canned_result_;
    }

    // --- Assert helpers ----------------------------------------------------

    // The options passed to the most recent authenticate() call.
    // std::nullopt if authenticate() has never been called.
    [[nodiscard]] std::optional<web_authenticator_options> last_options() const {
        return last_options_;
    }

    // Number of times authenticate() has been called.
    [[nodiscard]] int call_count() const noexcept { return call_count_; }

    // Reset recorded call state (does not affect the canned result).
    void reset_calls() {
        last_options_ = std::nullopt;
        call_count_   = 0;
    }

private:
    std::optional<web_authenticator_result>  canned_result_{};
    std::optional<web_authenticator_options> last_options_{};
    int                                      call_count_{ 0 };
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_WEB_AUTHENTICATOR_HPP
