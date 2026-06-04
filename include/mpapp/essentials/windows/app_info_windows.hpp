// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::windows_app_info` — Win32 system app-info backend.
// Implements `mpapp::app_info` using:
//   - GetModuleFileNameW for the executable path (package_name / name).
//   - GetFileVersionInfoSizeW / GetFileVersionInfoW / VerQueryValueW for
//     VS_FIXEDFILEINFO (version_string / build_string).
//   - RegGetValueW on HKCU\...\AppsUseLightTheme for requested_theme.
//   - ShellExecuteW for show_settings_ui (ms-settings:appsfeatures-app).
// No windows.h in this header; all Win32 details are confined to the .cpp.

#ifndef MPAPP_ESSENTIALS_WINDOWS_APP_INFO_WINDOWS_HPP
#define MPAPP_ESSENTIALS_WINDOWS_APP_INFO_WINDOWS_HPP

#include <string>

#include "../../essentials/app_info.hpp"

namespace mpapp {

// Win32 app-info backend. Implements `mpapp::app_info` using the Windows API.
// package_name() and name() return the basename of the executable (without
// extension) obtained from GetModuleFileNameW. version_string() returns the
// dotted "Major.Minor.Patch" string from the VERSIONINFO resource;
// build_string() returns the Build component. If the resource is absent,
// both return "0". requested_theme() reads AppsUseLightTheme from the
// registry and maps it to app_theme. show_settings_ui() opens
// ms-settings:appsfeatures-app via ShellExecuteW.
class windows_app_info final : public app_info {
public:
    windows_app_info()  = default;
    ~windows_app_info() = default;

    windows_app_info(const windows_app_info&)            = delete;
    windows_app_info& operator=(const windows_app_info&) = delete;
    windows_app_info(windows_app_info&&)                 = delete;
    windows_app_info& operator=(windows_app_info&&)      = delete;

    // Returns the basename (without .exe extension) of the running executable.
    [[nodiscard]] std::string package_name() const override;

    // Same as package_name() — the display name for Win32 apps without a
    // manifest-based display name.
    [[nodiscard]] std::string name() const override;

    // Returns "Major.Minor.Patch" from the embedded VERSIONINFO resource,
    // or "0.0.0" when the resource is absent.
    [[nodiscard]] std::string version_string() const override;

    // Returns the Build component of the VERSIONINFO resource as a decimal
    // string, or "0" when the resource is absent.
    [[nodiscard]] std::string build_string() const override;

    // Reads HKCU\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize
    // AppsUseLightTheme (DWORD).  1 -> light, 0 -> dark, absent -> unspecified.
    [[nodiscard]] app_theme requested_theme() const override;

    // Returns layout_direction::left_to_right unconditionally on Win32
    // (locale-based RTL detection is out of scope for this backend).
    [[nodiscard]] layout_direction requested_layout_direction() const override;

    // Attempts to open ms-settings:appsfeatures-app via ShellExecuteW.
    // This is a best-effort call; failure is silently ignored (no mock
    // tracking on the real backend).
    void show_settings_ui() override;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_WINDOWS_APP_INFO_WINDOWS_HPP
