// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::windows_device_display` — Windows Win32 backend for
// `mpapp::device_display`.  Uses GetSystemMetrics, GetDpiForSystem (or
// GetDeviceCaps fallback), and EnumDisplaySettings to populate display_info.
// windows.h is confined to the .cpp; this header is platform-neutral.

#ifndef MPAPP_ESSENTIALS_WINDOWS_DEVICE_DISPLAY_WINDOWS_HPP
#define MPAPP_ESSENTIALS_WINDOWS_DEVICE_DISPLAY_WINDOWS_HPP

#include "../../essentials/device_display.hpp"

namespace mpapp {

// Windows Win32 implementation of `mpapp::device_display`.
//
// On construction an initial read of the primary monitor metrics is performed.
// refresh() re-reads the metrics and emits main_display_info_changed whenever
// the snapshot differs from the previously cached one.
//
// Thread safety: not thread-safe. Call from a single thread or guard
// externally.
class windows_device_display final : public device_display {
public:
    // Constructs and performs an initial read of the system display metrics.
    windows_device_display();

    windows_device_display(const windows_device_display&)            = delete;
    windows_device_display& operator=(const windows_device_display&) = delete;
    windows_device_display(windows_device_display&&)                 = delete;
    windows_device_display& operator=(windows_device_display&&)      = delete;

    ~windows_device_display() override = default;

    // ---- device_display interface -------------------------------------------

    // Returns the cached display_info populated on construction or last
    // refresh().
    [[nodiscard]] display_info main_display_info() const override;

    // Returns whether the keep-screen-on flag is currently set.
    [[nodiscard]] bool keep_screen_on() const override;

    // Sets (or clears) the keep-screen-on flag.  When set to true, calls
    // SetThreadExecutionState to prevent the display from sleeping.
    void set_keep_screen_on(bool value) override;

    // Re-reads primary monitor metrics. Emits main_display_info_changed if
    // the snapshot has changed since the last read.
    void refresh();

private:
    // Cached snapshot updated on construction and each refresh().
    display_info info_{};
    bool         keep_screen_on_{ false };

    // Performs the Win32 read and updates info_.
    void read_and_update();
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_WINDOWS_DEVICE_DISPLAY_WINDOWS_HPP
