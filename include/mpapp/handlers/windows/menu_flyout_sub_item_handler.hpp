// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 `basic_menu_flyout_sub_item` handler — wraps
// `mux::Controls::MenuFlyoutSubItem`. `text` flows to `.Text()`;
// `items` rebuild the submenu's Items collection by resolving each
// child `view*` through the ADR-0013 dispatch registry.

#ifndef MPAPP_HANDLERS_WINDOWS_MENU_FLYOUT_SUB_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_MENU_FLYOUT_SUB_ITEM_HANDLER_HPP

#include <string>
#include <vector>

#include "../../internal/basic_menu_flyout_sub_item.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class menu_flyout_sub_item_handler<platform::windows> {
public:
    menu_flyout_sub_item_handler();
    ~menu_flyout_sub_item_handler();
    menu_flyout_sub_item_handler(const menu_flyout_sub_item_handler&)            = delete;
    menu_flyout_sub_item_handler& operator=(const menu_flyout_sub_item_handler&) = delete;
    menu_flyout_sub_item_handler(menu_flyout_sub_item_handler&&)                 = delete;
    menu_flyout_sub_item_handler& operator=(menu_flyout_sub_item_handler&&)      = delete;

    void map_text(basic_menu_flyout_sub_item& s);
    void map_items(basic_menu_flyout_sub_item& s);

    winrt::Microsoft::UI::Xaml::Controls::MenuFlyoutSubItem&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::MenuFlyoutSubItem& native() const noexcept { return native_; }

private:
    void apply_text(const std::string& v);
    void apply_items(const std::vector<view*>& v);

    struct text_cb_t  { menu_flyout_sub_item_handler<platform::windows>* self; void operator()(const std::string& v)        const { self->apply_text(v); } };
    struct items_cb_t { menu_flyout_sub_item_handler<platform::windows>* self; void operator()(const std::vector<view*>& v) const { self->apply_items(v); } };

    winrt::Microsoft::UI::Xaml::Controls::MenuFlyoutSubItem native_{nullptr};

    text_cb_t                                  text_cb_{this};
    items_cb_t                                 items_cb_{this};
    signal_slot<const std::string&>            text_slot_{};
    signal_slot<std::vector<view*> const&>     items_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_MENU_FLYOUT_SUB_ITEM_HANDLER_HPP
