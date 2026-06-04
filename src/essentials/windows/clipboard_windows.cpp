// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Win32 implementation of `mpapp::windows_clipboard`.
// windows.h is confined to this translation unit.

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <stdexcept>
#include <string>

#include "mpapp/essentials/windows/clipboard_windows.hpp"

namespace {

// Convert a UTF-8 std::string to a UTF-16 std::wstring.
// Returns an empty wstring for empty input.
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

// Convert a null-terminated UTF-16 wide string to a UTF-8 std::string.
[[nodiscard]] std::string utf16_to_utf8(const wchar_t* utf16, int len)
{
    if (len <= 0) {
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
    std::string utf8(static_cast<std::size_t>(required), '\0');
    ::WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS,
        utf16, len,
        utf8.data(), required,
        nullptr, nullptr);
    return utf8;
}

// RAII wrapper: opens the clipboard in its constructor and closes it in its
// destructor. Calling open() may fail; callers must check is_open().
class scoped_clipboard {
public:
    explicit scoped_clipboard(HWND owner = nullptr) noexcept
        : open_{ ::OpenClipboard(owner) != FALSE }
    {}

    ~scoped_clipboard() noexcept {
        if (open_) {
            ::CloseClipboard();
        }
    }

    scoped_clipboard(const scoped_clipboard&)            = delete;
    scoped_clipboard& operator=(const scoped_clipboard&) = delete;
    scoped_clipboard(scoped_clipboard&&)                 = delete;
    scoped_clipboard& operator=(scoped_clipboard&&)      = delete;

    [[nodiscard]] bool is_open() const noexcept { return open_; }

private:
    bool open_;
};

// RAII wrapper: acquires a GlobalLock on construction, releases on destruction.
class scoped_global_lock {
public:
    explicit scoped_global_lock(HGLOBAL handle) noexcept
        : handle_{ handle }
        , ptr_{ handle ? ::GlobalLock(handle) : nullptr }
    {}

    ~scoped_global_lock() noexcept {
        if (ptr_) {
            ::GlobalUnlock(handle_);
        }
    }

    scoped_global_lock(const scoped_global_lock&)            = delete;
    scoped_global_lock& operator=(const scoped_global_lock&) = delete;
    scoped_global_lock(scoped_global_lock&&)                 = delete;
    scoped_global_lock& operator=(scoped_global_lock&&)      = delete;

    [[nodiscard]] void* get() const noexcept { return ptr_; }

private:
    HGLOBAL handle_;
    void*   ptr_;
};

} // anonymous namespace

namespace mpapp {

void windows_clipboard::set_text(const std::string& text)
{
    scoped_clipboard cb;
    if (!cb.is_open()) {
        // Cannot open clipboard — emit false (content unchanged / unknown).
        clipboard_content_changed.emit(false);
        return;
    }

    if (!::EmptyClipboard()) {
        clipboard_content_changed.emit(false);
        return;
    }

    if (text.empty()) {
        // Clipboard is now empty; emit cleared notification.
        clipboard_content_changed.emit(false);
        return;
    }

    const std::wstring utf16 = utf8_to_utf16(text);
    // Size in bytes: (length + 1) wide chars for the null terminator.
    const std::size_t byte_count =
        (utf16.size() + 1u) * sizeof(wchar_t);

    HGLOBAL hmem = ::GlobalAlloc(GMEM_MOVEABLE, byte_count);
    if (!hmem) {
        clipboard_content_changed.emit(false);
        return;
    }

    {
        scoped_global_lock lock{ hmem };
        if (!lock.get()) {
            ::GlobalFree(hmem);
            clipboard_content_changed.emit(false);
            return;
        }
        auto* dest = static_cast<wchar_t*>(lock.get());
        // Copy text + null terminator.
        std::copy(utf16.begin(), utf16.end(), dest);
        dest[utf16.size()] = L'\0';
    }

    if (!::SetClipboardData(CF_UNICODETEXT, hmem)) {
        // SetClipboardData failed; we must free the handle ourselves.
        ::GlobalFree(hmem);
        clipboard_content_changed.emit(false);
        return;
    }
    // Ownership of hmem transferred to the clipboard on success; do not free.

    clipboard_content_changed.emit(true);
}

std::optional<std::string> windows_clipboard::get_text() const
{
    if (!::IsClipboardFormatAvailable(CF_UNICODETEXT)) {
        return std::nullopt;
    }

    scoped_clipboard cb;
    if (!cb.is_open()) {
        return std::nullopt;
    }

    HANDLE hdata = ::GetClipboardData(CF_UNICODETEXT);
    if (!hdata) {
        return std::nullopt;
    }

    scoped_global_lock lock{ static_cast<HGLOBAL>(hdata) };
    const auto* utf16 = static_cast<const wchar_t*>(lock.get());
    if (!utf16) {
        return std::nullopt;
    }

    // Determine the string length (excluding the null terminator).
    int len = 0;
    while (utf16[len] != L'\0') {
        ++len;
    }

    if (len == 0) {
        return std::nullopt;
    }

    std::string result = utf16_to_utf8(utf16, len);
    if (result.empty()) {
        return std::nullopt;
    }
    return result;
}

bool windows_clipboard::has_text() const
{
    return ::IsClipboardFormatAvailable(CF_UNICODETEXT) != FALSE;
}

} // namespace mpapp
