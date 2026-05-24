// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 `basic_menu_flyout_separator` handler — wraps
// `mux::Controls::MenuFlyoutSeparator`. Carries no observable
// properties; the native widget is constructed once and exposed via
// `native()` for the basic_menu_flyout handler to pack into its Items
// collection.

#ifndef MPAPP_HANDLERS_WINDOWS_MENU_FLYOUT_SEPARATOR_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_MENU_FLYOUT_SEPARATOR_HANDLER_HPP

#include "../../internal/basic_menu_flyout_separator.hpp"
#include "../../platform.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class menu_flyout_separator_handler<platform::windows> {
public:
    menu_flyout_separator_handler();
    ~menu_flyout_separator_handler();
    menu_flyout_separator_handler(const menu_flyout_separator_handler&)            = delete;
    menu_flyout_separator_handler& operator=(const menu_flyout_separator_handler&) = delete;
    menu_flyout_separator_handler(menu_flyout_separator_handler&&)                 = delete;
    menu_flyout_separator_handler& operator=(menu_flyout_separator_handler&&)      = delete;

    winrt::Microsoft::UI::Xaml::Controls::MenuFlyoutSeparator&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::MenuFlyoutSeparator& native() const noexcept { return native_; }

private:
    winrt::Microsoft::UI::Xaml::Controls::MenuFlyoutSeparator native_{nullptr};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_MENU_FLYOUT_SEPARATOR_HANDLER_HPP
