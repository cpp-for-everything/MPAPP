// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Win32 implementation of `mpapp::windows_launcher` and
// `mpapp::windows_browser`.
// windows.h is confined to this translation unit.

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>

#include <string>
#include <string_view>

#include "mpapp/essentials/windows/launcher_windows.hpp"

namespace {

// Convert a UTF-8 std::string to a UTF-16 std::wstring.
// Returns an empty wstring for empty input or on conversion error.
[[nodiscard]] std::wstring utf8_to_utf16(const std::string& utf8)
{
    if (utf8.empty()) {
        return {};
    }
    const int required = ::MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS,
        utf8.data(), static_cast<int>(utf8.size()),
        nullptr, 0);
    if (required <= 0) {
        return {};
    }
    std::wstring utf16(static_cast<std::size_t>(required), L'\0');
    ::MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS,
        utf8.data(), static_cast<int>(utf8.size()),
        utf16.data(), required);
    return utf16;
}

// Returns true when `uri` starts with `scheme_prefix` (case-insensitive ASCII
// comparison up to the length of the prefix).
[[nodiscard]] bool starts_with_scheme(std::string_view uri,
                                      std::string_view scheme_prefix) noexcept
{
    if (uri.size() < scheme_prefix.size()) {
        return false;
    }
    for (std::size_t i = 0; i < scheme_prefix.size(); ++i) {
        // Lowercase the URI character before comparing.
        const char c = (uri[i] >= 'A' && uri[i] <= 'Z')
                       ? static_cast<char>(uri[i] + ('a' - 'A'))
                       : uri[i];
        if (c != scheme_prefix[i]) {
            return false;
        }
    }
    return true;
}

// Returns true for URI schemes that Windows reliably handles out of the box.
[[nodiscard]] bool is_known_scheme(std::string_view uri) noexcept
{
    return starts_with_scheme(uri, "http://")   ||
           starts_with_scheme(uri, "https://")  ||
           starts_with_scheme(uri, "mailto:")   ||
           starts_with_scheme(uri, "tel:")       ||
           starts_with_scheme(uri, "file://")   ||
           starts_with_scheme(uri, "file:///");
}

// Invoke ShellExecuteW with verb L"open" for the given UTF-8 URI.
// Returns the raw HINSTANCE return value (compare > 32 for success).
// Returns 0 for an empty URI.
[[nodiscard]] INT_PTR shell_open(const std::string& uri)
{
    if (uri.empty()) {
        return 0;
    }
    const std::wstring wuri = utf8_to_utf16(uri);
    if (wuri.empty()) {
        return 0;
    }
    // ShellExecuteW returns HINSTANCE (a pointer-sized value on Win64).
    // Values > 32 indicate success per Win32 docs.
    return reinterpret_cast<INT_PTR>(
        ::ShellExecuteW(nullptr, L"open", wuri.c_str(), nullptr, nullptr, SW_SHOWNORMAL));
}

// Compile-time checks: both classes must be instantiatable (no abstract leftover).
static_assert(sizeof(mpapp::windows_launcher) > 0,
              "windows_launcher must be a complete type");
static_assert(sizeof(mpapp::windows_browser) > 0,
              "windows_browser must be a complete type");

} // anonymous namespace

namespace mpapp {

// ---- windows_launcher -------------------------------------------------------

bool windows_launcher::can_open(const std::string& uri) const
{
    if (uri.empty()) {
        return false;
    }
    // For known schemes return true immediately without touching the OS.
    if (is_known_scheme(uri)) {
        return true;
    }
    // For any other non-empty URI be permissive — ShellExecuteW will handle
    // the "no handler" case at open time. This matches MAUI's lenient approach.
    return true;
}

bool windows_launcher::try_open(const std::string& uri)
{
    if (uri.empty()) {
        return false;
    }
    const INT_PTR result = shell_open(uri);
    return result > 32;
}

void windows_launcher::open(const std::string& uri)
{
    if (uri.empty()) {
        return;
    }
    (void)shell_open(uri); // return value intentionally discarded
}

// ---- windows_browser --------------------------------------------------------

bool windows_browser::open(const std::string& uri)
{
    // Win32 has no in-app browser; always delegate to ShellExecuteW.
    const INT_PTR result = shell_open(uri);
    return result > 32;
}

bool windows_browser::open(const std::string& uri,
                             browser_launch_mode /*mode*/)
{
    // Launch mode is advisory on Win32; ignore it.
    const INT_PTR result = shell_open(uri);
    return result > 32;
}

bool windows_browser::open(const std::string& uri,
                             const browser_launch_options& /*options*/)
{
    // Options (mode, title_mode) are advisory on Win32; ignore them.
    const INT_PTR result = shell_open(uri);
    return result > 32;
}

} // namespace mpapp
