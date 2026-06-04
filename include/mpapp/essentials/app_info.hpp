// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::app_info` — application identity, version, and theme metadata.
// Counterpart to MAUI Essentials `AppInfo` + `AppTheme`. Abstract interface
// + a settable mock implementation for tests. `app_theme` and
// `layout_direction` are defined here and are reused by other framework
// components (e.g. windowing, theming engine).
//
// `show_settings_ui()` opens the platform's per-app settings page; the mock
// records whether it was called via `settings_ui_shown()`.
//
// No macros; header-only interface. Conforms to ADR-0002.

#ifndef MPAPP_ESSENTIALS_APP_INFO_HPP
#define MPAPP_ESSENTIALS_APP_INFO_HPP

#include <cstdint>
#include <string>
#include <string_view>

#include "../signal.hpp"

namespace mpapp {

// ---- Enumerations ----------------------------------------------------------

// Mirrors MAUI AppTheme.  `unspecified` means the host OS did not report a
// preference (e.g. older API levels) — framework components should fall back
// to their own default in that case.
enum class app_theme : std::uint8_t {
    unspecified = 0,
    light       = 1,
    dark        = 2,
};

// Mirrors MAUI LayoutDirection.
enum class layout_direction : std::uint8_t {
    unknown       = 0,
    left_to_right = 1,
    right_to_left = 2,
};

// ---- to_string helpers (constexpr, no macros) ------------------------------

[[nodiscard]] constexpr std::string_view to_string(app_theme t) noexcept {
    switch (t) {
        case app_theme::light:       return "light";
        case app_theme::dark:        return "dark";
        case app_theme::unspecified: return "unspecified";
        default:                     return "?";
    }
}

[[nodiscard]] constexpr std::string_view to_string(layout_direction d) noexcept {
    switch (d) {
        case layout_direction::left_to_right: return "left_to_right";
        case layout_direction::right_to_left: return "right_to_left";
        case layout_direction::unknown:       return "unknown";
        default:                              return "?";
    }
}

// ---- Abstract interface ----------------------------------------------------

class app_info {
public:
    virtual ~app_info() = default;

    // Identity and version strings as reported by the platform manifest.
    [[nodiscard]] virtual std::string package_name()    const = 0;
    [[nodiscard]] virtual std::string name()            const = 0;
    [[nodiscard]] virtual std::string version_string()  const = 0;
    [[nodiscard]] virtual std::string build_string()    const = 0;

    // The user-requested UI theme (OS preference).
    [[nodiscard]] virtual app_theme requested_theme()            const = 0;

    // The active layout direction derived from the locale.
    [[nodiscard]] virtual layout_direction requested_layout_direction() const = 0;

    // Opens the platform settings page for this application (e.g. iOS
    // Settings.app or Android per-app settings).  May be a no-op on
    // platforms that do not expose such a page.
    virtual void show_settings_ui() = 0;

    // Fires when the OS notifies the app that the user's theme preference
    // has changed (e.g. switching system-wide between light and dark).
    mpapp::signal<app_theme> requested_theme_changed{};
};

// ---- Mock / in-memory implementation ---------------------------------------

// All state is settable so tests can drive the interface without a real
// platform.  Setting `requested_theme` to a different value emits the
// `requested_theme_changed` signal.  `show_settings_ui()` is a no-op that
// records its invocation via `settings_ui_shown()`.
class mock_app_info final : public app_info {
public:
    mock_app_info() = default;

    // --- Getters (interface) -------------------------------------------------

    [[nodiscard]] std::string package_name()   const override { return package_name_; }
    [[nodiscard]] std::string name()           const override { return name_; }
    [[nodiscard]] std::string version_string() const override { return version_string_; }
    [[nodiscard]] std::string build_string()   const override { return build_string_; }

    [[nodiscard]] app_theme requested_theme() const override {
        return theme_;
    }

    [[nodiscard]] layout_direction requested_layout_direction() const override {
        return layout_direction_;
    }

    void show_settings_ui() override {
        settings_ui_shown_ = true;
    }

    // --- Setters (mock-specific) ---------------------------------------------

    void set_package_name(std::string v)   { package_name_   = std::move(v); }
    void set_name(std::string v)           { name_            = std::move(v); }
    void set_version_string(std::string v) { version_string_ = std::move(v); }
    void set_build_string(std::string v)   { build_string_   = std::move(v); }

    // Setting the same theme is a no-op (no signal fired).
    void set_requested_theme(app_theme t) {
        if (t == theme_) {
            return;
        }
        theme_ = t;
        requested_theme_changed.emit(t);
    }

    void set_requested_layout_direction(layout_direction d) {
        layout_direction_ = d;
    }

    // --- Observation helpers -------------------------------------------------

    // Returns true if `show_settings_ui()` has been called at least once.
    [[nodiscard]] bool settings_ui_shown() const noexcept {
        return settings_ui_shown_;
    }

    // Resets the `settings_ui_shown` flag (useful between test assertions).
    void reset_settings_ui_shown() noexcept {
        settings_ui_shown_ = false;
    }

private:
    std::string     package_name_   = "com.example.app";
    std::string     name_           = "Example App";
    std::string     version_string_ = "1.0.0";
    std::string     build_string_   = "1";
    app_theme       theme_          = app_theme::unspecified;
    layout_direction layout_direction_ = layout_direction::left_to_right;
    bool            settings_ui_shown_ = false;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_APP_INFO_HPP
