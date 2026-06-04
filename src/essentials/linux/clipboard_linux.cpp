// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// GTK4 GdkClipboard implementation of `mpapp::linux_clipboard`.
// All GDK/GTK headers are confined to this translation unit; no GDK types
// leak into the public header.
//
// Null-display safety: gdk_display_get_default() returns null in headless
// environments (CI, WSL without a Wayland/X11 session). Every GDK call is
// guarded so the backend compiles and runs safely without a display.
//
// get_text() — synchronous best-effort:
//   GdkClipboard reads are inherently asynchronous (gdk_clipboard_read_text_async
//   + gdk_clipboard_read_text_finish with a GMainLoop spin). For this backend we
//   cache the last value written via set_text() and return it synchronously.
//   Cross-process async reads (reading text set by another application) are a
//   follow-up tracked in RFC-0013; they require either running a GMainLoop or
//   bridging to a thread pool.

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include <optional>
#include <string>

#include "mpapp/essentials/linux/clipboard_linux.hpp"

namespace mpapp {

void linux_clipboard::set_text(const std::string& text)
{
    GdkDisplay* display = gdk_display_get_default();
    if (display == nullptr) {
        // Headless / no display — degrade silently.
        clipboard_content_changed.emit(false);
        return;
    }

    GdkClipboard* cb = gdk_display_get_clipboard(display);
    if (cb == nullptr) {
        clipboard_content_changed.emit(false);
        return;
    }

    if (text.empty()) {
        // Clear the clipboard content.
        gdk_clipboard_set_text(cb, "");
        cached_text_.clear();
        clipboard_content_changed.emit(false);
        return;
    }

    gdk_clipboard_set_text(cb, text.c_str());
    cached_text_ = text;
    clipboard_content_changed.emit(true);
}

std::optional<std::string> linux_clipboard::get_text() const
{
    // Synchronous best-effort: return the last value set by this backend.
    // Cross-process async reads via gdk_clipboard_read_text_async are a
    // follow-up (RFC-0013). A full async implementation would spin a
    // GMainLoop on a dedicated thread or integrate with an application event
    // loop, which is out of scope here.
    if (cached_text_.empty()) {
        return std::nullopt;
    }
    return cached_text_;
}

bool linux_clipboard::has_text() const
{
    // For the synchronous cache path, mirror get_text() semantics.
    // If a display is available we can also query GdkClipboard formats for a
    // more accurate answer, but that requires an async content-type fetch too.
    // Keep it consistent with get_text(): report based on the local cache.
    return !cached_text_.empty();
}

} // namespace mpapp

#endif // defined(__linux__) && !defined(__ANDROID__)
