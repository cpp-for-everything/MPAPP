// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::browser` — in-app / system browser launcher. Counterpart to MAUI
// Essentials `Browser`. Abstract interface + an in-memory mock implementation
// that records the last URI and options passed to `open()` and returns a
// test-settable result flag (default true). Real per-platform backends
// (Windows ShellExecute / WebView2, Linux xdg-open, Android Intent.ACTION_VIEW,
// iOS SFSafariViewController) implement the same interface and are injected via
// the DI container (RFC-0011). No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_BROWSER_HPP
#define MPAPP_ESSENTIALS_BROWSER_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace mpapp {

// Mirrors MAUI's BrowserLaunchMode.
enum class browser_launch_mode : std::uint8_t {
    system_preferred = 0,  // let the OS pick the best in-app or external browser
    external         = 1,  // always use the external/default system browser
};

// Mirrors MAUI's BrowserTitleMode.
enum class browser_title_mode : std::uint8_t {
    default_ = 0,  // platform default title bar behaviour
    show     = 1,  // always show the URL / page title in the browser chrome
    hide     = 2,  // hide the title bar / URL bar
};

// ---- to_string helpers (constexpr, no macros) --------------------------------

[[nodiscard]] constexpr std::string_view to_string(browser_launch_mode m) noexcept {
    switch (m) {
        case browser_launch_mode::system_preferred: return "system_preferred";
        case browser_launch_mode::external:         return "external";
        default:                                    return "?";
    }
}

[[nodiscard]] constexpr std::string_view to_string(browser_title_mode m) noexcept {
    switch (m) {
        case browser_title_mode::default_: return "default";
        case browser_title_mode::show:     return "show";
        case browser_title_mode::hide:     return "hide";
        default:                           return "?";
    }
}

// ---- Options value type ------------------------------------------------------

struct browser_launch_options {
    browser_launch_mode mode       = browser_launch_mode::system_preferred;
    browser_title_mode  title_mode = browser_title_mode::default_;

    bool operator==(const browser_launch_options&) const = default;
};

// ---- Abstract interface ------------------------------------------------------

class browser {
public:
    virtual ~browser() = default;

    // Open a URI using platform defaults (system_preferred, default title mode).
    [[nodiscard]] virtual bool open(const std::string& uri) = 0;

    // Open a URI with a specific launch mode (title mode stays default).
    [[nodiscard]] virtual bool open(const std::string& uri,
                                    browser_launch_mode mode) = 0;

    // Open a URI with fully specified options.
    [[nodiscard]] virtual bool open(const std::string& uri,
                                    const browser_launch_options& options) = 0;
};

// ---- Mock / in-memory implementation -----------------------------------------
//
// * `set_result(false)` simulates a platform where opening a browser fails
//   (e.g. sandbox restrictions, invalid URI).  All three `open()` overloads
//   respect this flag.
// * `last_uri()` returns the URI passed to the most recent `open()` call, or
//   `std::nullopt` if `open()` has never been called.
// * `last_options()` returns the fully-resolved options passed to (or derived
//   from) the most recent `open()` call, or `std::nullopt` if never called.
// * `open_count()` counts every call regardless of the result flag.
// * `reset()` clears recorded state without altering the result flag.

class mock_browser final : public browser {
public:
    explicit mock_browser(bool result = true) noexcept : result_{ result } {}

    // ---- browser interface ---------------------------------------------------

    [[nodiscard]] bool open(const std::string& uri) override {
        return record(uri, browser_launch_options{});
    }

    [[nodiscard]] bool open(const std::string& uri,
                            browser_launch_mode mode) override {
        browser_launch_options opts;
        opts.mode = mode;
        return record(uri, opts);
    }

    [[nodiscard]] bool open(const std::string& uri,
                            const browser_launch_options& options) override {
        return record(uri, options);
    }

    // ---- Test-control helpers -----------------------------------------------

    // Set the value that every open() call will return.
    void set_result(bool r) noexcept { result_ = r; }

    // URI passed to the most recent open() call, or nullopt if never called.
    [[nodiscard]] std::optional<std::string> last_uri() const noexcept {
        return last_uri_;
    }

    // Fully-resolved options passed to the most recent open() call, or nullopt.
    [[nodiscard]] std::optional<browser_launch_options> last_options() const noexcept {
        return last_options_;
    }

    // Total number of open() calls made (regardless of result).
    [[nodiscard]] int open_count() const noexcept { return open_count_; }

    // Reset recorded state (last_uri, last_options, open_count) without
    // altering the result flag.
    void reset() noexcept {
        last_uri_     = std::nullopt;
        last_options_ = std::nullopt;
        open_count_   = 0;
    }

private:
    bool record(const std::string& uri, const browser_launch_options& opts) {
        last_uri_     = uri;
        last_options_ = opts;
        ++open_count_;
        return result_;
    }

    bool                                    result_      = true;
    std::optional<std::string>              last_uri_{};
    std::optional<browser_launch_options>   last_options_{};
    int                                     open_count_  = 0;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_BROWSER_HPP
