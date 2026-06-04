// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Windows Win32 implementation of `mpapp::windows_device_display`.
// windows.h is included here and nowhere else.

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstring>

#include "mpapp/essentials/windows/device_display_windows.hpp"

namespace {

// ---------------------------------------------------------------------------
// DPI helpers
// ---------------------------------------------------------------------------

// GetDpiForSystem is available on Windows 8.1+ (shcore.dll).  We attempt a
// dynamic load so the binary stays runnable on Windows 7 (where the function
// does not exist) and falls back to GetDeviceCaps(LOGPIXELSX) via a
// screen DC.
[[nodiscard]] double query_dpi() noexcept
{
    // Try GetDpiForSystem first (Win8.1+).
    using PFN_GetDpiForSystem = UINT (WINAPI*)();
    HMODULE user32 = ::GetModuleHandleW(L"user32.dll");
    if (user32) {
        // Cast via void* to silence -Wcast-function-type; the Win32 ABI
        // guarantees that FARPROC is an acceptable intermediate for function
        // pointer round-trips via GetProcAddress.
        FARPROC raw = ::GetProcAddress(user32, "GetDpiForSystem");
        PFN_GetDpiForSystem fn = nullptr;
        if (raw) {
            // Two-step cast: FARPROC -> void* -> target fn-ptr type.
            // This is well-defined for re-interpreting the underlying bits
            // on all Win32 platforms (calling convention and pointer size
            // are both fixed).
            void* vp = reinterpret_cast<void*>(raw);
            std::memcpy(&fn, &vp, sizeof(fn));
        }
        if (fn) {
            const UINT dpi = fn();
            if (dpi > 0) {
                return static_cast<double>(dpi) / 96.0;
            }
        }
    }

    // Fallback: GetDeviceCaps on the screen DC.
    HDC screen = ::GetDC(nullptr);
    if (screen) {
        const int logpixels = ::GetDeviceCaps(screen, LOGPIXELSX);
        ::ReleaseDC(nullptr, screen);
        if (logpixels > 0) {
            return static_cast<double>(logpixels) / 96.0;
        }
    }

    return 1.0; // safe default
}

// ---------------------------------------------------------------------------
// Orientation / rotation
// ---------------------------------------------------------------------------

[[nodiscard]] mpapp::display_orientation
orientation_from_dimensions(double width, double height) noexcept
{
    if (width <= 0.0 || height <= 0.0) {
        return mpapp::display_orientation::unknown;
    }
    return (width >= height)
        ? mpapp::display_orientation::landscape
        : mpapp::display_orientation::portrait;
}

// EnumDisplaySettings uses DEVMODE.dmDisplayOrientation:
//   DMDO_DEFAULT (0) = 0°
//   DMDO_90      (1) = 90°
//   DMDO_180     (2) = 180°
//   DMDO_270     (3) = 270°
[[nodiscard]] mpapp::display_rotation query_rotation() noexcept
{
    DEVMODEW dm{};
    dm.dmSize = static_cast<WORD>(sizeof(dm));
    if (::EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &dm) &&
        (dm.dmFields & DM_DISPLAYORIENTATION))
    {
        switch (dm.dmDisplayOrientation) {
            case DMDO_90:  return mpapp::display_rotation::rotation_90;
            case DMDO_180: return mpapp::display_rotation::rotation_180;
            case DMDO_270: return mpapp::display_rotation::rotation_270;
            default:       return mpapp::display_rotation::rotation_0;
        }
    }
    return mpapp::display_rotation::rotation_0;
}

// ---------------------------------------------------------------------------
// Refresh rate
// ---------------------------------------------------------------------------

[[nodiscard]] double query_refresh_rate() noexcept
{
    DEVMODEW dm{};
    dm.dmSize = static_cast<WORD>(sizeof(dm));
    if (::EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &dm) &&
        (dm.dmFields & DM_DISPLAYFREQUENCY) &&
        dm.dmDisplayFrequency > 1)
    {
        return static_cast<double>(dm.dmDisplayFrequency);
    }
    return 60.0; // safe default
}

// ---------------------------------------------------------------------------
// Full snapshot read
// ---------------------------------------------------------------------------

[[nodiscard]] mpapp::display_info read_display_info() noexcept
{
    mpapp::display_info info{};

    info.width   = static_cast<double>(::GetSystemMetrics(SM_CXSCREEN));
    info.height  = static_cast<double>(::GetSystemMetrics(SM_CYSCREEN));
    info.density = query_dpi();
    info.rate    = query_refresh_rate();
    info.rotation    = query_rotation();
    info.orientation = orientation_from_dimensions(info.width, info.height);

    return info;
}

} // anonymous namespace

namespace mpapp {

windows_device_display::windows_device_display()
{
    read_and_update();
}

display_info windows_device_display::main_display_info() const
{
    return info_;
}

bool windows_device_display::keep_screen_on() const
{
    return keep_screen_on_;
}

void windows_device_display::set_keep_screen_on(bool value)
{
    keep_screen_on_ = value;
    if (value) {
        // Prevent the display from turning off for the lifetime of this flag.
        ::SetThreadExecutionState(
            ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
    } else {
        // Clear the override; revert to normal power-management behaviour.
        ::SetThreadExecutionState(ES_CONTINUOUS);
    }
}

void windows_device_display::refresh()
{
    read_and_update();
}

void windows_device_display::read_and_update()
{
    const display_info next = read_display_info();
    if (next == info_) {
        return;
    }
    info_ = next;
    main_display_info_changed.emit(info_);
}

} // namespace mpapp
