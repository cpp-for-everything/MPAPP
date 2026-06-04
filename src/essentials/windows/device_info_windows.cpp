// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Win32 implementation of `mpapp::windows_device_info`.
// windows.h is confined to this translation unit.

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// ntdll RtlGetVersion — available on all Win32 platforms (XP+).
// We load it at runtime via GetProcAddress to avoid a hard link on ntdll's
// import lib and to stay compatible with MinGW toolchains that may not
// export RTL_OSVERSIONINFOW directly.
#include <winnt.h>   // RTL_OSVERSIONINFOW
#include <winreg.h>  // RegOpenKeyExW / RegQueryValueExW / RegCloseKey

#include <cstdio>
#include <string>

#include "mpapp/essentials/windows/device_info_windows.hpp"

namespace {

// ---------------------------------------------------------------------------
// UTF-16 -> UTF-8 helper (same pattern as clipboard_windows.cpp)
// ---------------------------------------------------------------------------
[[nodiscard]] std::string utf16_to_utf8(const wchar_t* utf16, int len)
{
    if (!utf16 || len <= 0) {
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

[[nodiscard]] std::string wstring_to_utf8(const std::wstring& ws)
{
    if (ws.empty()) {
        return {};
    }
    return utf16_to_utf8(ws.data(), static_cast<int>(ws.size()));
}

// ---------------------------------------------------------------------------
// OS version via RtlGetVersion (ntdll.dll)
// Using GetProcAddress avoids ntdll import lib dependency and works on MinGW.
// ---------------------------------------------------------------------------
[[nodiscard]] std::string query_os_version()
{
    using RtlGetVersionFn = LONG (WINAPI*)(PRTL_OSVERSIONINFOW);

    HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) {
        return "unknown";
    }

    // GetProcAddress returns FARPROC whose signature intentionally differs from
    // the real function pointer type; the cast is the canonical Win32 idiom.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
    auto rtl_get_version = reinterpret_cast<RtlGetVersionFn>(  // NOLINT
        ::GetProcAddress(ntdll, "RtlGetVersion"));
#pragma GCC diagnostic pop
    if (!rtl_get_version) {
        return "unknown";
    }

    RTL_OSVERSIONINFOW osvi{};
    osvi.dwOSVersionInfoSize = static_cast<DWORD>(sizeof(osvi));
    if (rtl_get_version(&osvi) != 0 /* STATUS_SUCCESS = 0 */) {
        return "unknown";
    }

    char buf[64]{};
    std::snprintf(buf, sizeof(buf), "%lu.%lu.%lu",
        osvi.dwMajorVersion,
        osvi.dwMinorVersion,
        osvi.dwBuildNumber);
    return buf;
}

// ---------------------------------------------------------------------------
// Read a REG_SZ / REG_EXPAND_SZ registry string value.
// Returns empty string on any error.
// ---------------------------------------------------------------------------
[[nodiscard]] std::string read_registry_string(
    HKEY root,
    const wchar_t* subkey,
    const wchar_t* value_name)
{
    HKEY hkey = nullptr;
    if (::RegOpenKeyExW(root, subkey, 0, KEY_READ, &hkey) != ERROR_SUCCESS) {
        return {};
    }

    DWORD type  = 0;
    DWORD bytes = 0;
    // First call: get required buffer size.
    LONG rc = ::RegQueryValueExW(hkey, value_name, nullptr, &type, nullptr, &bytes);
    if (rc != ERROR_SUCCESS && rc != ERROR_MORE_DATA) {
        ::RegCloseKey(hkey);
        return {};
    }
    if (type != REG_SZ && type != REG_EXPAND_SZ) {
        ::RegCloseKey(hkey);
        return {};
    }

    // bytes may not include the null terminator depending on how the value
    // was stored; add two bytes (one wide char) to be safe.
    const std::size_t wchar_count = (bytes / sizeof(wchar_t)) + 1u;
    std::wstring buf(wchar_count, L'\0');

    rc = ::RegQueryValueExW(
        hkey, value_name, nullptr, &type,
        reinterpret_cast<LPBYTE>(buf.data()),  // NOLINT
        &bytes);
    ::RegCloseKey(hkey);

    if (rc != ERROR_SUCCESS) {
        return {};
    }

    // Trim trailing null wide chars before converting.
    while (!buf.empty() && buf.back() == L'\0') {
        buf.pop_back();
    }
    return wstring_to_utf8(buf);
}

// ---------------------------------------------------------------------------
// Manufacturer: try OEM BIOS string stored by Windows in the registry.
// Key: HKLM\HARDWARE\DESCRIPTION\System\BIOS, value: SystemManufacturer
// Fallback: "Unknown"
// ---------------------------------------------------------------------------
[[nodiscard]] std::string query_manufacturer()
{
    std::string manufacturer = read_registry_string(
        HKEY_LOCAL_MACHINE,
        L"HARDWARE\\DESCRIPTION\\System\\BIOS",
        L"SystemManufacturer");

    if (!manufacturer.empty()) {
        return manufacturer;
    }
    return "Unknown";
}

// ---------------------------------------------------------------------------
// Model: try SystemProductName from the same BIOS registry key.
// Fallback: "PC"
// ---------------------------------------------------------------------------
[[nodiscard]] std::string query_model()
{
    std::string model = read_registry_string(
        HKEY_LOCAL_MACHINE,
        L"HARDWARE\\DESCRIPTION\\System\\BIOS",
        L"SystemProductName");

    if (!model.empty()) {
        return model;
    }
    return "PC";
}

} // anonymous namespace

namespace mpapp {

device_info windows_device_info()
{
    device_info info;
    info.platform     = device_platform::windows;
    info.idiom        = device_idiom::desktop;
    info.model        = query_model();
    info.manufacturer = query_manufacturer();
    info.version      = query_os_version();
    return info;
}

} // namespace mpapp

#endif // defined(_WIN32)
