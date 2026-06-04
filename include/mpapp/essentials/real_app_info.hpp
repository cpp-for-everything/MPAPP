// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::real_app_info` — a REAL, cross-platform backend for the
// `mpapp::app_info` interface, written once and compiled into every
// target (no ifdefs): static identity metadata (package_name, name,
// version_string, build_string) is supplied at construction time by the
// application entry-point (on MAUI these values are read from the platform
// manifest; on desktop they are passed in from a compile-time constant or a
// configuration file).
//
// Theme detection: `requested_theme` defaults to `app_theme::unspecified`
// and can be overridden via `set_requested_theme()`.  Full OS-level theme
// probing is deferred to per-platform follow-up work and will live in
// separate platform-specific compilation units so that this header stays
// dependency-free and host-verifiable.
//
// Layout direction defaults to `layout_direction::left_to_right` and is
// similarly settable for locale-aware callers.
//
// `show_settings_ui()` records intent via `settings_ui_shown()` (a no-op on
// platforms that have no per-app settings page); real platform dispatch will
// be wired in platform-specific backends.
//
// No macros in the public API. Header-only. Conforms to ADR-0002.

#ifndef MPAPP_ESSENTIALS_REAL_APP_INFO_HPP
#define MPAPP_ESSENTIALS_REAL_APP_INFO_HPP

#include <string>
#include <string_view>

#include "app_info.hpp"

namespace mpapp {

// REAL, cross-platform `app_info` backend.
//
// Construct with the four static metadata strings the app knows at startup.
// Sensible defaults are provided for each so that a default-constructed
// instance is immediately usable in test scaffolding without supplying all
// four arguments.
//
// Rule of Zero: all members are value types; no explicit destructor, copy,
// or move is declared.
class real_app_info final : public app_info {
public:
    // Construct with static manifest metadata.  All arguments have defaults
    // so the class is usable with partial construction (e.g. in tests).
    explicit real_app_info(
        std::string package_name   = "com.example.app",
        std::string name           = "Example App",
        std::string version_string = "1.0.0",
        std::string build_string   = "1"
    )
        : package_name_  { std::move(package_name)   }
        , name_          { std::move(name)            }
        , version_string_{ std::move(version_string) }
        , build_string_  { std::move(build_string)   }
    {}

    // ---- app_info interface -------------------------------------------------

    [[nodiscard]] std::string package_name()   const override { return package_name_;   }
    [[nodiscard]] std::string name()           const override { return name_;           }
    [[nodiscard]] std::string version_string() const override { return version_string_; }
    [[nodiscard]] std::string build_string()   const override { return build_string_;   }

    [[nodiscard]] app_theme requested_theme() const override {
        return theme_;
    }

    [[nodiscard]] layout_direction requested_layout_direction() const override {
        return layout_direction_;
    }

    // Records that the caller requested the platform settings UI.
    // Real platform dispatch is wired in platform-specific backends.
    void show_settings_ui() override {
        settings_ui_shown_ = true;
    }

    // ---- Setters ------------------------------------------------------------

    // Override the detected (or default) theme.  Setting a different value
    // emits `requested_theme_changed`; setting the same value is a no-op.
    void set_requested_theme(app_theme t) {
        if (t == theme_) {
            return;
        }
        theme_ = t;
        requested_theme_changed.emit(t);
    }

    // Override the active layout direction (e.g. after a locale change).
    void set_requested_layout_direction(layout_direction d) {
        layout_direction_ = d;
    }

    // ---- Observation helpers ------------------------------------------------

    // Returns true if `show_settings_ui()` has been called at least once.
    [[nodiscard]] bool settings_ui_shown() const noexcept {
        return settings_ui_shown_;
    }

    // Resets the `settings_ui_shown` flag (useful between test assertions).
    void reset_settings_ui_shown() noexcept {
        settings_ui_shown_ = false;
    }

private:
    std::string      package_name_;
    std::string      name_;
    std::string      version_string_;
    std::string      build_string_;
    app_theme        theme_            = app_theme::unspecified;
    layout_direction layout_direction_ = layout_direction::left_to_right;
    bool             settings_ui_shown_ = false;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_REAL_APP_INFO_HPP
