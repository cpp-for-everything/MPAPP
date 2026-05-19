// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Internal helper — not part of the public include/ surface.
//
// UTF-8 -> winrt::hstring conversion shared by the Windows handlers.
// Lives in src/ so it cannot leak into user code; the public surface is
// always std::string at the cross-platform boundary.

#ifndef MPAPP_SRC_HANDLERS_WINDOWS_WINRT_STRINGS_HPP
#define MPAPP_SRC_HANDLERS_WINDOWS_WINRT_STRINGS_HPP

#include <windows.h>

#include <string>
#include <string_view>

#include <winrt/base.h>

namespace mpapp::detail {

inline winrt::hstring to_hstring_utf8(std::string_view utf8) {
    if (utf8.empty()) {
        return {};
    }
    const int required = ::MultiByteToWideChar(
        CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()),
        nullptr, 0);
    if (required <= 0) {
        return {};
    }
    std::wstring wide(static_cast<std::size_t>(required), L'\0');
    ::MultiByteToWideChar(
        CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()),
        wide.data(), required);
    return winrt::hstring{wide};
}

inline std::string wstring_to_utf8(std::wstring_view wide) {
    if (wide.empty()) {
        return {};
    }
    const int required = ::WideCharToMultiByte(
        CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()),
        nullptr, 0, nullptr, nullptr);
    if (required <= 0) {
        return {};
    }
    std::string utf8(static_cast<std::size_t>(required), '\0');
    ::WideCharToMultiByte(
        CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()),
        utf8.data(), required, nullptr, nullptr);
    return utf8;
}

} // namespace mpapp::detail

#endif // MPAPP_SRC_HANDLERS_WINDOWS_WINRT_STRINGS_HPP
