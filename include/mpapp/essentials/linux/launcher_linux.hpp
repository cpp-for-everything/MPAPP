// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::linux_launcher` — Linux GIO backend for `mpapp::launcher`.
// `mpapp::linux_browser`  — Linux GIO backend for `mpapp::browser`.
//
// Both classes delegate to GIO (g_app_info_launch_default_for_uri) confined
// entirely to the .cpp translation unit. This header is platform-neutral:
// it includes only the two interface headers and standard C++ headers.
// No <gtk/gtk.h> or <gio/gio.h> here.
//
// linux_launcher:
//   can_open(uri)  — returns true for well-known schemes (http, https, mailto,
//                    tel, file). For other schemes uses
//                    g_app_info_get_default_for_uri_scheme to query whether a
//                    handler is registered.
//   try_open(uri)  — calls g_app_info_launch_default_for_uri; returns true on
//                    success (gboolean TRUE).
//   open(uri)      — calls g_app_info_launch_default_for_uri; ignores result.
//
// linux_browser:
//   open(uri)               — delegates to GIO; Linux has no in-app browser.
//   open(uri, mode)         — same; launch mode is advisory, ignored on Linux.
//   open(uri, options)      — same; options are advisory, ignored on Linux.
//
// Thread safety: not thread-safe. Call from a single thread or guard externally.

#ifndef MPAPP_ESSENTIALS_LINUX_LAUNCHER_LINUX_HPP
#define MPAPP_ESSENTIALS_LINUX_LAUNCHER_LINUX_HPP

#include <string>

#include "../../essentials/browser.hpp"
#include "../../essentials/launcher.hpp"

namespace mpapp {

// Linux GIO implementation of `mpapp::launcher`.
//
// can_open() returns true immediately for the common URI schemes (http, https,
// mailto, tel, file). For all other schemes it queries GIO for a registered
// handler; returns false only when GIO reports no handler exists.
//
// try_open() / open() both call g_app_info_launch_default_for_uri.
// try_open returns false for empty URIs or when GIO reports failure.
//
// Thread safety: not thread-safe. Call from a single thread or guard externally.
class linux_launcher final : public launcher {
public:
    linux_launcher()  = default;
    ~linux_launcher() override = default;

    linux_launcher(const linux_launcher&)            = delete;
    linux_launcher& operator=(const linux_launcher&) = delete;
    linux_launcher(linux_launcher&&)                 = delete;
    linux_launcher& operator=(linux_launcher&&)      = delete;

    // Returns true for http, https, mailto, tel, and file schemes.
    // For other schemes, queries GIO for a registered handler.
    [[nodiscard]] bool can_open(const std::string& uri) const override;

    // Attempts to open `uri` via GIO. Returns true on success.
    // Returns false for empty URIs or on GIO failure.
    bool try_open(const std::string& uri) override;

    // Opens `uri` via GIO. Return value is discarded.
    void open(const std::string& uri) override;
};

// Linux GIO implementation of `mpapp::browser`.
//
// Linux has no in-app browser primitive; all three open() overloads delegate
// to g_app_info_launch_default_for_uri. The launch mode and title mode in
// browser_launch_options are advisory and are silently ignored on Linux.
//
// Thread safety: not thread-safe. Call from a single thread or guard externally.
class linux_browser final : public browser {
public:
    linux_browser()  = default;
    ~linux_browser() override = default;

    linux_browser(const linux_browser&)            = delete;
    linux_browser& operator=(const linux_browser&) = delete;
    linux_browser(linux_browser&&)                 = delete;
    linux_browser& operator=(linux_browser&&)      = delete;

    // Open `uri` using platform defaults (GIO; no in-app browser).
    [[nodiscard]] bool open(const std::string& uri) override;

    // Open `uri` with the given launch mode (ignored on Linux).
    [[nodiscard]] bool open(const std::string& uri,
                            browser_launch_mode mode) override;

    // Open `uri` with fully specified options (options ignored on Linux).
    [[nodiscard]] bool open(const std::string& uri,
                            const browser_launch_options& options) override;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_LINUX_LAUNCHER_LINUX_HPP
