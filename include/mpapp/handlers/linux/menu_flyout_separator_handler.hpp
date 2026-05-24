// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 `basic_menu_flyout_separator` handler — wraps a
// horizontal `GtkSeparator`. No observable properties; the native
// widget is constructed once and exposed via `native()` for the
// containing basic_menu_flyout to pack into its box.

#ifndef MPAPP_HANDLERS_LINUX_MENU_FLYOUT_SEPARATOR_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_MENU_FLYOUT_SEPARATOR_HANDLER_HPP

#include "../../internal/basic_menu_flyout_separator.hpp"
#include "../../platform.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class menu_flyout_separator_handler<platform::linux_> {
public:
    menu_flyout_separator_handler();
    ~menu_flyout_separator_handler();
    menu_flyout_separator_handler(const menu_flyout_separator_handler&)            = delete;
    menu_flyout_separator_handler& operator=(const menu_flyout_separator_handler&) = delete;
    menu_flyout_separator_handler(menu_flyout_separator_handler&&)                 = delete;
    menu_flyout_separator_handler& operator=(menu_flyout_separator_handler&&)      = delete;

    void*       native() noexcept       { return native_; }  // GtkSeparator*
    const void* native() const noexcept { return native_; }

private:
    void* native_ = nullptr;  // GtkSeparator*
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_MENU_FLYOUT_SEPARATOR_HANDLER_HPP
