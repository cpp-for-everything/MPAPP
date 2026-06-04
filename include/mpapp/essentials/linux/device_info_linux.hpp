// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::linux_device_info` — Linux desktop device-info backend.
// Reads OS version from uname()/​/etc/os-release and DMI sysfs entries
// to fill an `mpapp::device_info` value type.
// No OS-specific headers appear here; all Linux details are confined to
// the .cpp translation unit.

#ifndef MPAPP_ESSENTIALS_LINUX_DEVICE_INFO_LINUX_HPP
#define MPAPP_ESSENTIALS_LINUX_DEVICE_INFO_LINUX_HPP

#include "mpapp/essentials/device_info.hpp"

namespace mpapp {

// Factory: query Linux sysfs / uname / /etc/os-release and return a filled
// `device_info` struct.
//
//   platform     — device_platform::linux_ (always)
//   idiom        — device_idiom::desktop (always for Linux desktop)
//   model        — /sys/devices/virtual/dmi/id/product_name if readable,
//                  otherwise "Linux"
//   manufacturer — /sys/devices/virtual/dmi/id/sys_vendor if readable,
//                  otherwise "Unknown"
//   version      — PRETTY_NAME from /etc/os-release if available,
//                  otherwise "major.minor.patch" from uname()
//
// This function is only defined when compiled on Linux (excluding Android).
// The header is always parseable on any platform — the implementation guard
// lives in the .cpp.
[[nodiscard]] device_info linux_device_info();

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_LINUX_DEVICE_INFO_LINUX_HPP
