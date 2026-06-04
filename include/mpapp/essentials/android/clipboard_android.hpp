// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::android_clipboard` — Android system clipboard backend.
// Implements `mpapp::clipboard` using android.content.ClipboardManager,
// reached through the app Context obtained from the JNI bridge
// (mpapp::detail::get_activity()). All JNI details (<jni.h>, FindClass,
// CallObjectMethod, DeleteLocalRef, AttachCurrentThread) are confined to
// the .cpp translation unit; this header stays JNI-free, mirroring the
// Windows backend. No macros in the public API.

#ifndef MPAPP_ESSENTIALS_ANDROID_CLIPBOARD_ANDROID_HPP
#define MPAPP_ESSENTIALS_ANDROID_CLIPBOARD_ANDROID_HPP

#include <optional>
#include <string>

#include "../../essentials/clipboard.hpp"

namespace mpapp {

// Android clipboard backend. Implements `mpapp::clipboard` via
// android.content.ClipboardManager, obtained from the app Context with
// Context.getSystemService(Context.CLIPBOARD_SERVICE).
//
// The Context is taken from the JNI bridge (detail::get_activity()), which
// the host MainActivity sets once during native init. The bridge exposes a
// Context accessor, so no Context is required in the constructor.
//
// set_text()  -> ClipData.newPlainText(label, text) + setPrimaryClip,
//                then emits clipboard_content_changed (false when empty).
// get_text()  -> getPrimaryClip().getItemAt(0).getText(); std::nullopt when
//                there is no primary clip or no text item.
// has_text()  -> hasPrimaryClip() plus a MIME-type check for plain/HTML text.
class android_clipboard final : public clipboard {
public:
    android_clipboard()  = default;
    ~android_clipboard() = default;

    android_clipboard(const android_clipboard&)            = delete;
    android_clipboard& operator=(const android_clipboard&) = delete;
    android_clipboard(android_clipboard&&)                 = delete;
    android_clipboard& operator=(android_clipboard&&)      = delete;

    // Write UTF-8 text to the Android clipboard as a plain-text ClipData.
    // Emits clipboard_content_changed(true) on non-empty text, or
    // clipboard_content_changed(false) when text is empty.
    void set_text(const std::string& text) override;

    // Read UTF-8 text from the Android clipboard's primary clip (item 0).
    // Returns std::nullopt when there is no primary clip, no text item, or
    // the clipboard service / Context is unavailable.
    [[nodiscard]] std::optional<std::string> get_text() const override;

    // Returns true when the clipboard has a primary clip whose description
    // reports a text MIME type (text/plain or text/html).
    [[nodiscard]] bool has_text() const override;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_ANDROID_CLIPBOARD_ANDROID_HPP
