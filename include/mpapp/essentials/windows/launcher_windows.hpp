// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::windows_launcher` — Windows Win32 backend for `mpapp::launcher`.
// `mpapp::windows_browser` — Windows Win32 backend for `mpapp::browser`.
//
// Both classes delegate to ShellExecuteW (shellapi.h / shell32.dll) confined
// entirely to the .cpp translation unit. This header is platform-neutral:
// it includes only the two interface headers and standard C++ headers.
// No windows.h here.
//
// windows_launcher:
//   can_open(uri)  — returns true for well-known schemes (http, https, mailto,
//                    tel, file) and best-effort for any other registered scheme.
//   try_open(uri)  — calls ShellExecuteW; returns true when the return value
//                    is greater than 32 (Win32 success convention).
//   open(uri)      — calls ShellExecuteW; ignores return value.
//
// windows_browser:
//   open(uri)               — calls ShellExecuteW; Win32 has no in-app browser.
//   open(uri, mode)         — same; launch mode is advisory, ignored on Win32.
//   open(uri, options)      — same; options are advisory, ignored on Win32.
//
// UTF-8 <-> UTF-16 conversion uses MultiByteToWideChar / WideCharToMultiByte
// (CP_UTF8), implemented in the .cpp.

#ifndef MPAPP_ESSENTIALS_WINDOWS_LAUNCHER_WINDOWS_HPP
#define MPAPP_ESSENTIALS_WINDOWS_LAUNCHER_WINDOWS_HPP

#include <string>

#include "../../essentials/browser.hpp"
#include "../../essentials/launcher.hpp"

namespace mpapp {

// Windows Win32 implementation of `mpapp::launcher`.
//
// can_open() returns true for the common URI schemes (http, https, mailto,
// tel, file). For all other schemes it performs a conservative best-effort
// check and returns true, matching MAUI's lenient behaviour on platforms that
// don't expose a synchronous scheme-query API.
//
// try_open() / open() both call ShellExecuteW with the verb L"open".
// ShellExecuteW return value > 32 is the Win32 convention for success.
//
// Thread safety: not thread-safe. Call from a single thread or guard externally.
class windows_launcher final : public launcher {
public:
    windows_launcher()  = default;
    ~windows_launcher() override = default;

    windows_launcher(const windows_launcher&)            = delete;
    windows_launcher& operator=(const windows_launcher&) = delete;
    windows_launcher(windows_launcher&&)                 = delete;
    windows_launcher& operator=(windows_launcher&&)      = delete;

    // Returns true for http, https, mailto, tel, and file schemes.
    // Returns true for any other non-empty URI (best-effort / permissive).
    [[nodiscard]] bool can_open(const std::string& uri) const override;

    // Attempts to open `uri` via ShellExecuteW. Returns true on Win32 success
    // (return value > 32). Returns false for empty URIs or on failure.
    bool try_open(const std::string& uri) override;

    // Opens `uri` via ShellExecuteW. Return value is discarded.
    void open(const std::string& uri) override;
};

// Windows Win32 implementation of `mpapp::browser`.
//
// Win32 has no in-app browser primitive; all three open() overloads delegate
// to ShellExecuteW with L"open". The launch mode and title mode in
// browser_launch_options are advisory and are silently ignored on Windows.
//
// Thread safety: not thread-safe. Call from a single thread or guard externally.
class windows_browser final : public browser {
public:
    windows_browser()  = default;
    ~windows_browser() override = default;

    windows_browser(const windows_browser&)            = delete;
    windows_browser& operator=(const windows_browser&) = delete;
    windows_browser(windows_browser&&)                 = delete;
    windows_browser& operator=(windows_browser&&)      = delete;

    // Open `uri` using platform defaults (ShellExecuteW; no in-app browser).
    [[nodiscard]] bool open(const std::string& uri) override;

    // Open `uri` with the given launch mode (ignored on Win32).
    [[nodiscard]] bool open(const std::string& uri,
                            browser_launch_mode mode) override;

    // Open `uri` with fully specified options (options ignored on Win32).
    [[nodiscard]] bool open(const std::string& uri,
                            const browser_launch_options& options) override;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_WINDOWS_LAUNCHER_WINDOWS_HPP
