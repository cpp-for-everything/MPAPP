// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::windows_device_info` — Win32 device-info backend.
// Queries the OS version (RtlGetVersion), OEM manufacturer string, and
// screen geometry to fill an `mpapp::device_info` value type.
// No windows.h in this header; all Win32 details are confined to the .cpp
// translation unit.

#ifndef MPAPP_ESSENTIALS_WINDOWS_DEVICE_INFO_WINDOWS_HPP
#define MPAPP_ESSENTIALS_WINDOWS_DEVICE_INFO_WINDOWS_HPP

#include "mpapp/essentials/device_info.hpp"

namespace mpapp {

// Factory: query the Win32 APIs and return a filled `device_info` struct.
//
//   platform  — device_platform::windows (always)
//   idiom     — device_idiom::desktop (always for Win32)
//   model     — "PC" (best-effort; no universal Win32 API for model name)
//   manufacturer — OEM name from HKLM\…\SystemInformation if available,
//                  otherwise "Unknown"
//   version   — "major.minor.build" from RtlGetVersion (ntdll.dll)
//
// This function is only defined when compiled on Windows (_WIN32). The
// header is always parseable on any platform — the implementation guard
// lives in the .cpp.
[[nodiscard]] device_info windows_device_info();

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_WINDOWS_DEVICE_INFO_WINDOWS_HPP
