// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 `menu_flyout_item` handler — wraps
// `mux::Controls::MenuFlyoutItem`. `text` flows through to `.Text()`;
// `is_enabled` to `.IsEnabled()`. The native item's `Click` event
// fires the cross-platform `clicked` signal.

#ifndef MPAPP_HANDLERS_WINDOWS_MENU_FLYOUT_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_MENU_FLYOUT_ITEM_HANDLER_HPP

#include <string>

#include "../../menu_flyout_item.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class menu_flyout_item_handler<platform::windows> {
public:
    menu_flyout_item_handler();
    ~menu_flyout_item_handler();
    menu_flyout_item_handler(const menu_flyout_item_handler&)            = delete;
    menu_flyout_item_handler& operator=(const menu_flyout_item_handler&) = delete;
    menu_flyout_item_handler(menu_flyout_item_handler&&)                 = delete;
    menu_flyout_item_handler& operator=(menu_flyout_item_handler&&)      = delete;

    void map_text(menu_flyout_item& i);
    void map_is_enabled(menu_flyout_item& i);

    winrt::Microsoft::UI::Xaml::Controls::MenuFlyoutItem&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::MenuFlyoutItem& native() const noexcept { return native_; }

private:
    void apply_text(const std::string& v);
    void apply_is_enabled(bool v);

    struct text_cb_t       { menu_flyout_item_handler<platform::windows>* self; void operator()(const std::string& v) const { self->apply_text(v); } };
    struct is_enabled_cb_t { menu_flyout_item_handler<platform::windows>* self; void operator()(bool v)               const { self->apply_is_enabled(v); } };

    winrt::Microsoft::UI::Xaml::Controls::MenuFlyoutItem native_{nullptr};
    menu_flyout_item*                                    owner_ = nullptr;
    winrt::event_token                                   click_token_{};

    text_cb_t                          text_cb_{this};
    is_enabled_cb_t                    is_enabled_cb_{this};
    signal_slot<const std::string&>    text_slot_{};
    signal_slot<const bool&>           is_enabled_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_MENU_FLYOUT_ITEM_HANDLER_HPP
