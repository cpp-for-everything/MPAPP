// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Win32 implementation of `mpapp::windows_app_info`.
// windows.h (and version.h, shellapi.h) are confined to this translation unit.

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#include <winreg.h>

// winver.h declares GetFileVersionInfoSizeW / GetFileVersionInfoW /
// VerQueryValueW.  Link with -lversion (MinGW) or version.lib (MSVC).
#include <winver.h>

#include <memory>
#include <string>
#include <vector>

#include "mpapp/essentials/windows/app_info_windows.hpp"

namespace {

// ---------------------------------------------------------------------------
// UTF-16 <-> UTF-8 helpers
// ---------------------------------------------------------------------------

[[nodiscard]] std::string utf16_to_utf8(const wchar_t* utf16, int len)
{
    if (!utf16 || len <= 0) {
        return {};
    }
    const int required = ::WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS,
        utf16, len,
        nullptr, 0,
        nullptr, nullptr);
    if (required <= 0) {
        return {};
    }
    std::string result(static_cast<std::size_t>(required), '\0');
    ::WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS,
        utf16, len,
        result.data(), required,
        nullptr, nullptr);
    return result;
}

[[nodiscard]] std::string wstr_to_utf8(const std::wstring& ws)
{
    return utf16_to_utf8(ws.data(), static_cast<int>(ws.size()));
}

// ---------------------------------------------------------------------------
// Exe path helper
// ---------------------------------------------------------------------------

// Returns the full path to the running executable as a wide string.
[[nodiscard]] std::wstring exe_path_wide()
{
    std::wstring buf(MAX_PATH, L'\0');
    for (;;) {
        DWORD len = ::GetModuleFileNameW(
            nullptr,
            buf.data(),
            static_cast<DWORD>(buf.size()));

        if (len == 0) {
            return {};
        }
        if (len < static_cast<DWORD>(buf.size())) {
            buf.resize(len);
            return buf;
        }
        // Buffer was too small; double it.
        buf.resize(buf.size() * 2, L'\0');
    }
}

// Extracts the filename stem (basename without extension) from a full path.
[[nodiscard]] std::wstring stem_from_path(const std::wstring& path)
{
    // Find last backslash or forward slash.
    const auto sep = path.find_last_of(L"\\/");
    std::wstring filename = (sep == std::wstring::npos)
        ? path
        : path.substr(sep + 1);

    // Strip extension.
    const auto dot = filename.rfind(L'.');
    if (dot != std::wstring::npos) {
        filename.erase(dot);
    }
    return filename;
}

// ---------------------------------------------------------------------------
// VERSIONINFO resource helper
// ---------------------------------------------------------------------------

struct version_quad {
    unsigned short major   = 0;
    unsigned short minor   = 0;
    unsigned short patch   = 0;
    unsigned short build   = 0;
    bool           present = false;
};

[[nodiscard]] version_quad read_version_resource(const std::wstring& path)
{
    version_quad result;

    const DWORD info_size = ::GetFileVersionInfoSizeW(path.c_str(), nullptr);
    if (info_size == 0) {
        return result;
    }

    std::vector<std::byte> buf(static_cast<std::size_t>(info_size));
    if (!::GetFileVersionInfoW(path.c_str(), 0, info_size, buf.data())) {
        return result;
    }

    VS_FIXEDFILEINFO* ffi = nullptr;
    UINT ffi_len          = 0;
    if (!::VerQueryValueW(buf.data(), L"\\",
                          reinterpret_cast<LPVOID*>(&ffi), &ffi_len)) {
        return result;
    }
    if (!ffi || ffi_len < sizeof(VS_FIXEDFILEINFO)) {
        return result;
    }
    if (ffi->dwSignature != 0xFEEF04BD) {
        return result;
    }

    result.major   = HIWORD(ffi->dwFileVersionMS);
    result.minor   = LOWORD(ffi->dwFileVersionMS);
    result.patch   = HIWORD(ffi->dwFileVersionLS);
    result.build   = LOWORD(ffi->dwFileVersionLS);
    result.present = true;
    return result;
}

// ---------------------------------------------------------------------------
// Registry theme helper
// ---------------------------------------------------------------------------

[[nodiscard]] mpapp::app_theme read_apps_light_theme()
{
    DWORD value     = 0;
    DWORD value_len = sizeof(value);

    const LSTATUS status = ::RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"AppsUseLightTheme",
        RRF_RT_REG_DWORD,
        nullptr,
        &value,
        &value_len);

    if (status != ERROR_SUCCESS) {
        return mpapp::app_theme::unspecified;
    }
    return (value != 0) ? mpapp::app_theme::light : mpapp::app_theme::dark;
}

} // anonymous namespace

namespace mpapp {

// ---------------------------------------------------------------------------
// windows_app_info — interface implementation
// ---------------------------------------------------------------------------

std::string windows_app_info::package_name() const
{
    const std::wstring path = exe_path_wide();
    if (path.empty()) {
        return {};
    }
    return wstr_to_utf8(stem_from_path(path));
}

std::string windows_app_info::name() const
{
    return package_name();
}

std::string windows_app_info::version_string() const
{
    const std::wstring path = exe_path_wide();
    if (path.empty()) {
        return "0.0.0";
    }
    const version_quad v = read_version_resource(path);
    if (!v.present) {
        return "0.0.0";
    }
    return std::to_string(v.major) + '.' +
           std::to_string(v.minor) + '.' +
           std::to_string(v.patch);
}

std::string windows_app_info::build_string() const
{
    const std::wstring path = exe_path_wide();
    if (path.empty()) {
        return "0";
    }
    const version_quad v = read_version_resource(path);
    if (!v.present) {
        return "0";
    }
    return std::to_string(v.build);
}

app_theme windows_app_info::requested_theme() const
{
    return read_apps_light_theme();
}

layout_direction windows_app_info::requested_layout_direction() const
{
    return layout_direction::left_to_right;
}

void windows_app_info::show_settings_ui()
{
    // Best-effort: open the Windows Settings app-features page.
    // ShellExecuteW returns an HINSTANCE cast; values > 32 indicate success.
    ::ShellExecuteW(
        nullptr,
        L"open",
        L"ms-settings:appsfeatures-app",
        nullptr,
        nullptr,
        SW_SHOWNORMAL);
}

} // namespace mpapp

#endif // defined(_WIN32)
