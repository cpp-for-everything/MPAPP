// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::launcher` — URI-based app/browser launcher. Counterpart to MAUI
// Essentials `Launcher`. Abstract interface + a mock/in-memory implementation
// suitable for unit tests. Real per-platform backends (Windows ShellExecute,
// Linux xdg-open, Android Intent, iOS UIApplication openURL) implement the
// same interface and are injected via the DI container (RFC-0011).
//
// API summary:
//   can_open(uri)  — returns true if the platform can handle the given URI.
//   try_open(uri)  — attempts to open; returns false if not openable.
//   open(uri)      — opens unconditionally (platform decides outcome).
//
// The mock records the most recent URI passed to try_open/open and exposes
// a configurable predicate so tests can control whether a URI is openable.

#ifndef MPAPP_ESSENTIALS_LAUNCHER_HPP
#define MPAPP_ESSENTIALS_LAUNCHER_HPP

#include <optional>
#include <string>
#include <unordered_set>

namespace mpapp {

// Abstract launcher interface.
class launcher {
public:
    virtual ~launcher() = default;

    // Returns true when the platform has a handler registered for `uri`.
    [[nodiscard]] virtual bool can_open(const std::string& uri) const = 0;

    // Attempts to open `uri`. Returns can_open(uri); records the call.
    virtual bool try_open(const std::string& uri) = 0;

    // Opens `uri` unconditionally. Records the call.
    virtual void open(const std::string& uri) = 0;
};

// Mock implementation: state is fully settable so tests can drive every path.
//
// Default behaviour:
//   - can_open returns true for all URIs unless a restricted set is configured.
//   - If open_whitelist is populated, only URIs in that set are openable.
//   - try_open records the URI and returns can_open(uri).
//   - open records the URI (no-op side-effect).
//   - last_opened() returns the most-recently opened URI (try_open or open).
class mock_launcher final : public launcher {
public:
    // Construct with the default allow-all behaviour.
    mock_launcher() = default;

    // Construct with an explicit whitelist of openable URIs.
    // Any URI NOT in this set will be considered not openable.
    explicit mock_launcher(std::unordered_set<std::string> openable_uris)
        : whitelist_{ std::move(openable_uris) }, use_whitelist_{ true } {}

    // ---- Interface implementation -------------------------------------------

    [[nodiscard]] bool can_open(const std::string& uri) const override {
        if (!use_whitelist_) {
            return true;
        }
        return whitelist_.count(uri) > 0;
    }

    bool try_open(const std::string& uri) override {
        bool result = can_open(uri);
        if (result) {
            last_opened_ = uri;
        }
        last_try_open_uri_ = uri;
        ++try_open_call_count_;
        return result;
    }

    void open(const std::string& uri) override {
        last_opened_ = uri;
        last_open_uri_ = uri;
        ++open_call_count_;
    }

    // ---- Test inspection API ------------------------------------------------

    // The URI most recently passed to a successful try_open or any open call.
    [[nodiscard]] std::optional<std::string> last_opened() const {
        return last_opened_;
    }

    // The URI most recently passed to try_open (regardless of success).
    [[nodiscard]] std::optional<std::string> last_try_open_uri() const {
        return last_try_open_uri_;
    }

    // The URI most recently passed to open.
    [[nodiscard]] std::optional<std::string> last_open_uri() const {
        return last_open_uri_;
    }

    // Number of times try_open has been called.
    [[nodiscard]] int try_open_call_count() const noexcept {
        return try_open_call_count_;
    }

    // Number of times open has been called.
    [[nodiscard]] int open_call_count() const noexcept {
        return open_call_count_;
    }

    // ---- Test control API ---------------------------------------------------

    // Add a URI to the whitelist (activates whitelist mode if not already on).
    void allow(const std::string& uri) {
        whitelist_.insert(uri);
        use_whitelist_ = true;
    }

    // Remove a URI from the whitelist.
    void disallow(const std::string& uri) {
        whitelist_.erase(uri);
    }

    // Switch to allow-all mode (whitelist is ignored).
    void allow_all() {
        use_whitelist_ = false;
    }

    // Reset all recorded state (but keep whitelist configuration).
    void reset_history() {
        last_opened_       = std::nullopt;
        last_try_open_uri_ = std::nullopt;
        last_open_uri_     = std::nullopt;
        try_open_call_count_ = 0;
        open_call_count_     = 0;
    }

private:
    std::unordered_set<std::string> whitelist_{};
    bool use_whitelist_{ false };

    std::optional<std::string> last_opened_{};
    std::optional<std::string> last_try_open_uri_{};
    std::optional<std::string> last_open_uri_{};
    int try_open_call_count_{ 0 };
    int open_call_count_{ 0 };
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_LAUNCHER_HPP
