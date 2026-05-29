// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::device_info` — static device metadata. Counterpart to MAUI
// Essentials `DeviceInfo`. A plain value type the per-platform layer
// fills in; `current_device_info()` returns a best-effort value derived
// from compile-time platform tags for the mock. No macros.

#ifndef MPAPP_ESSENTIALS_DEVICE_INFO_HPP
#define MPAPP_ESSENTIALS_DEVICE_INFO_HPP

#include <cstdint>
#include <string>

#include "../platform.hpp"

namespace mpapp {

enum class device_idiom : std::uint8_t {
    unknown = 0,
    phone   = 1,
    tablet  = 2,
    desktop = 3,
    tv      = 4,
    watch   = 5,
};

enum class device_platform : std::uint8_t {
    unknown = 0,
    windows = 1,
    android = 2,
    linux   = 3,
    macos   = 4,
    ios     = 5,
};

struct device_info {
    device_platform platform = device_platform::unknown;
    device_idiom    idiom    = device_idiom::unknown;
    std::string     model{};
    std::string     manufacturer{};
    std::string     version{};

    bool operator==(const device_info&) const = default;
};

// Best-effort device info from the compile-time platform tag. The real
// per-platform layer overrides model/manufacturer/version with native
// queries; this gives a sane default for the mock + host tools.
[[nodiscard]] inline device_info current_device_info() {
    device_info info;
    if constexpr (std::is_same_v<platform::current, platform::windows>) {
        info.platform = device_platform::windows;
        info.idiom    = device_idiom::desktop;
    } else if constexpr (std::is_same_v<platform::current, platform::android>) {
        info.platform = device_platform::android;
        info.idiom    = device_idiom::phone;
    } else if constexpr (std::is_same_v<platform::current, platform::linux_>) {
        info.platform = device_platform::linux;
        info.idiom    = device_idiom::desktop;
    } else if constexpr (std::is_same_v<platform::current, platform::macos>) {
        info.platform = device_platform::macos;
        info.idiom    = device_idiom::desktop;
    } else if constexpr (std::is_same_v<platform::current, platform::ios>) {
        info.platform = device_platform::ios;
        info.idiom    = device_idiom::phone;
    }
    return info;
}

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_DEVICE_INFO_HPP
