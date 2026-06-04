// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::linux_clipboard` — GTK4 GdkClipboard backend for Linux.
// Implements `mpapp::clipboard` using GdkClipboard obtained from the default
// GdkDisplay. All GDK/GTK headers are confined to the .cpp translation unit;
// this header exposes only the class declaration.
//
// NOTE: get_text() uses a synchronous best-effort strategy (last value set via
// this backend is cached and returned). Cross-process async reads via
// gdk_clipboard_read_text_async are a follow-up; see clipboard_linux.cpp.

#ifndef MPAPP_ESSENTIALS_LINUX_CLIPBOARD_LINUX_HPP
#define MPAPP_ESSENTIALS_LINUX_CLIPBOARD_LINUX_HPP

#include <optional>
#include <string>

#include "../../essentials/clipboard.hpp"

namespace mpapp {

// GTK4 clipboard backend. Implements `mpapp::clipboard` via GdkClipboard.
// set_text() calls gdk_clipboard_set_text() and emits clipboard_content_changed.
// get_text() / has_text() return data from the internal write-cache; cross-
// process reads require an async GDK round-trip (see .cpp for details).
// When gdk_display_get_default() returns null (headless / no display) all
// operations silently degrade: set_text is a no-op, get_text returns nullopt,
// has_text returns false.
class linux_clipboard final : public clipboard {
public:
    linux_clipboard()  = default;
    ~linux_clipboard() = default;

    linux_clipboard(const linux_clipboard&)            = delete;
    linux_clipboard& operator=(const linux_clipboard&) = delete;
    linux_clipboard(linux_clipboard&&)                 = delete;
    linux_clipboard& operator=(linux_clipboard&&)      = delete;

    // Write UTF-8 text to the system clipboard via GdkClipboard.
    // Emits clipboard_content_changed(true) on success, or
    // clipboard_content_changed(false) when text is empty or no display.
    void set_text(const std::string& text) override;

    // Return the last value written by this backend instance.
    // Cross-process clipboard reads (async GDK) are a follow-up.
    // Returns std::nullopt when nothing has been set or the last set was empty.
    [[nodiscard]] std::optional<std::string> get_text() const override;

    // Returns true when the last set_text call stored non-empty text.
    [[nodiscard]] bool has_text() const override;

private:
    // Cache of the last value pushed to GdkClipboard by this instance.
    std::string cached_text_{};
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_LINUX_CLIPBOARD_LINUX_HPP
