// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::clipboard` — text-based system clipboard access. Counterpart
// to MAUI Essentials `Clipboard`. Abstract interface + an in-memory
// mock implementation that is fully test-drivable (settable state, signal
// emission on change). Real per-platform backends (Windows Clipboard API,
// Linux xclip/wl-clipboard, Android ClipboardManager) implement the same
// interface and are injected via the DI container (RFC-0011). No macros;
// header-only interface.

#ifndef MPAPP_ESSENTIALS_CLIPBOARD_HPP
#define MPAPP_ESSENTIALS_CLIPBOARD_HPP

#include <optional>
#include <string>

#include "../signal.hpp"

namespace mpapp {

class clipboard {
public:
    virtual ~clipboard() = default;

    // Write text to the clipboard.
    virtual void set_text(const std::string& text) = 0;

    // Read text from the clipboard. Returns std::nullopt when the
    // clipboard is empty or contains non-text content.
    [[nodiscard]] virtual std::optional<std::string> get_text() const = 0;

    // Returns true when the clipboard contains text content.
    [[nodiscard]] virtual bool has_text() const = 0;

    // Fires whenever clipboard content changes. The bool payload is true
    // when the new content is non-empty text, false when cleared.
    mpapp::signal<bool> clipboard_content_changed{};
};

// Default + mock implementation: process-memory backed. set_text stores
// the value and emits clipboard_content_changed(true). Clearing via
// set_text("") sets the stored text to the empty string and emits
// clipboard_content_changed(false) since there is no text content.
class mock_clipboard final : public clipboard {
public:
    mock_clipboard() = default;

    void set_text(const std::string& text) override {
        text_ = text;
        clipboard_content_changed.emit(!text_.empty());
    }

    [[nodiscard]] std::optional<std::string> get_text() const override {
        if (text_.empty()) {
            return std::nullopt;
        }
        return text_;
    }

    [[nodiscard]] bool has_text() const override {
        return !text_.empty();
    }

private:
    std::string text_{};
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_CLIPBOARD_HPP
