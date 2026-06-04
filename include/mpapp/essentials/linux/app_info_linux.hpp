// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::linux_app_info` — GLib/GSettings backend for Linux.
// Implements `mpapp::app_info` using g_get_application_name(),
// g_get_prgname(), and GSettings to query the dark-theme preference.
// All GLib/GIO/GTK headers are confined to the .cpp translation unit;
// this header exposes only the class declaration.

#ifndef MPAPP_ESSENTIALS_LINUX_APP_INFO_LINUX_HPP
#define MPAPP_ESSENTIALS_LINUX_APP_INFO_LINUX_HPP

#include <cstdint>
#include <string>

#include "../../essentials/app_info.hpp"

namespace mpapp {

// GLib/GSettings backend for `mpapp::app_info` on Linux (non-Android).
//
// Identity fields:
//   name()         — g_get_application_name(), falls back to package_name()
//   package_name() — g_get_prgname(), falls back to /proc/self/comm, then ""
//   version_string() / build_string() — ctor-supplied, default ""
//
// Theme detection (best-effort, degrades gracefully):
//   1. Tries GSettings schema "org.freedesktop.appearance" key "color-scheme"
//      (0 = no preference, 1 = dark, 2 = light).
//   2. Falls back to GtkSettings property "gtk-application-prefer-dark-theme"
//      via gtk_settings_get_default() / g_object_get().
//   3. Returns app_theme::unspecified if neither source is available.
//
// show_settings_ui() is a no-op on Linux (no per-app system settings page).
//
// Thread safety: not thread-safe; call from a single thread or guard
// externally.
class linux_app_info final : public app_info {
public:
    // Construct with optional version/build strings.  Pass empty strings (the
    // default) when the values are not known at compile time.
    explicit linux_app_info(std::string version_string = {},
                            std::string build_string   = {});

    linux_app_info(const linux_app_info&)            = delete;
    linux_app_info& operator=(const linux_app_info&) = delete;
    linux_app_info(linux_app_info&&)                 = delete;
    linux_app_info& operator=(linux_app_info&&)      = delete;

    ~linux_app_info() override;

    // ---- mpapp::app_info interface ------------------------------------------

    [[nodiscard]] std::string package_name()   const override;
    [[nodiscard]] std::string name()           const override;
    [[nodiscard]] std::string version_string() const override;
    [[nodiscard]] std::string build_string()   const override;

    [[nodiscard]] app_theme        requested_theme()            const override;
    [[nodiscard]] layout_direction requested_layout_direction() const override;

    // No-op: Linux does not expose a per-app system settings page.
    void show_settings_ui() override;

private:
    std::string version_string_;
    std::string build_string_;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_LINUX_APP_INFO_LINUX_HPP
