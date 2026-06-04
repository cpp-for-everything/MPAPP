// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::windows_clipboard` — Win32 system clipboard backend.
// Implements `mpapp::clipboard` using the Windows Clipboard API
// (OpenClipboard / EmptyClipboard / SetClipboardData / GetClipboardData).
// UTF-8 <-> UTF-16 conversion is handled in the .cpp via
// MultiByteToWideChar / WideCharToMultiByte (CP_UTF8). No windows.h in
// this header; all Win32 details are confined to the .cpp translation unit.

#ifndef MPAPP_ESSENTIALS_WINDOWS_CLIPBOARD_WINDOWS_HPP
#define MPAPP_ESSENTIALS_WINDOWS_CLIPBOARD_WINDOWS_HPP

#include <optional>
#include <string>

#include "../../essentials/clipboard.hpp"

namespace mpapp {

// Win32 clipboard backend. Implements `mpapp::clipboard` via the Windows
// Clipboard API. set_text() converts the UTF-8 input to UTF-16, places it
// on the clipboard as CF_UNICODETEXT, then emits clipboard_content_changed.
// get_text() retrieves CF_UNICODETEXT and converts it back to UTF-8.
// has_text() calls IsClipboardFormatAvailable(CF_UNICODETEXT).
class windows_clipboard final : public clipboard {
public:
    windows_clipboard()  = default;
    ~windows_clipboard() = default;

    windows_clipboard(const windows_clipboard&)            = delete;
    windows_clipboard& operator=(const windows_clipboard&) = delete;
    windows_clipboard(windows_clipboard&&)                 = delete;
    windows_clipboard& operator=(windows_clipboard&&)      = delete;

    // Write UTF-8 text to the Win32 clipboard.
    // Emits clipboard_content_changed(true) on success, or
    // clipboard_content_changed(false) when text is empty.
    void set_text(const std::string& text) override;

    // Read text from the Win32 clipboard (CF_UNICODETEXT).
    // Returns std::nullopt when the clipboard contains no text content or
    // the clipboard cannot be opened.
    [[nodiscard]] std::optional<std::string> get_text() const override;

    // Returns true when CF_UNICODETEXT is currently available.
    [[nodiscard]] bool has_text() const override;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_WINDOWS_CLIPBOARD_WINDOWS_HPP
