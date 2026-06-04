// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Linux implementation of `mpapp::linux_device_info`.
// All Linux/POSIX headers are confined to this translation unit.

#include "mpapp/essentials/linux/device_info_linux.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <sys/utsname.h>  // uname / utsname

#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

namespace {

// ---------------------------------------------------------------------------
// Read the first non-empty line of a sysfs attribute file.
// Returns empty string on any I/O error or if the file does not exist.
// ---------------------------------------------------------------------------
[[nodiscard]] std::string read_sysfs_attr(const char* path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        return {};
    }
    std::string line;
    if (!std::getline(file, line)) {
        return {};
    }
    // Strip trailing whitespace / CR that sysfs sometimes includes.
    while (!line.empty() &&
           (line.back() == '\n' || line.back() == '\r' || line.back() == ' ')) {
        line.pop_back();
    }
    return line;
}

// ---------------------------------------------------------------------------
// OS version: try PRETTY_NAME from /etc/os-release first.
// The file uses KEY="value" or KEY=value shell assignment syntax.
// Falls back to "major.minor.patch" derived from uname() on failure.
// ---------------------------------------------------------------------------
[[nodiscard]] std::string query_os_version()
{
    // ---- /etc/os-release ----
    {
        std::ifstream file("/etc/os-release");
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                // Looking for:  PRETTY_NAME="Ubuntu 24.04 LTS"
                //           or: PRETTY_NAME=Ubuntu
                const char* prefix = "PRETTY_NAME=";
                const std::size_t prefix_len = std::strlen(prefix);
                if (line.size() <= prefix_len) {
                    continue;
                }
                if (line.compare(0, prefix_len, prefix) != 0) {
                    continue;
                }
                std::string value = line.substr(prefix_len);
                // Strip surrounding quotes if present.
                if (value.size() >= 2 &&
                    value.front() == '"' &&
                    value.back()  == '"') {
                    value = value.substr(1, value.size() - 2);
                }
                if (!value.empty()) {
                    return value;
                }
            }
        }
    }

    // ---- uname() fallback ----
    {
        struct utsname uts{};
        if (::uname(&uts) == 0) {
            // release is typically "6.5.0-28-generic" — return as-is.
            std::string release(uts.release);
            if (!release.empty()) {
                return release;
            }
        }
    }

    return "unknown";
}

// ---------------------------------------------------------------------------
// Manufacturer: /sys/devices/virtual/dmi/id/sys_vendor
// Fallback: "Unknown"
// ---------------------------------------------------------------------------
[[nodiscard]] std::string query_manufacturer()
{
    std::string vendor =
        read_sysfs_attr("/sys/devices/virtual/dmi/id/sys_vendor");
    if (!vendor.empty()) {
        return vendor;
    }
    return "Unknown";
}

// ---------------------------------------------------------------------------
// Model: /sys/devices/virtual/dmi/id/product_name
// Fallback: "Linux"
// ---------------------------------------------------------------------------
[[nodiscard]] std::string query_model()
{
    std::string product =
        read_sysfs_attr("/sys/devices/virtual/dmi/id/product_name");
    if (!product.empty()) {
        return product;
    }
    return "Linux";
}

} // anonymous namespace

namespace mpapp {

device_info linux_device_info()
{
    device_info info;
    info.platform     = device_platform::linux;
    info.idiom        = device_idiom::desktop;
    info.model        = query_model();
    info.manufacturer = query_manufacturer();
    info.version      = query_os_version();
    return info;
}

} // namespace mpapp

#endif // defined(__linux__) && !defined(__ANDROID__)
