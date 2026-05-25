// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_menu_bar_item handler — wraps
// `mux::Controls::MenuBarItem`.
//
// `title` maps onto `MenuBarItem::Title`. `items` rebuilds the basic_entry's
// `Items` collection from its child views resolved via the ADR-0013
// dispatch registry; concrete `MenuFlyoutItem*` types from the M-04c
// basic_menu_flyout family will slot in there. For now the rebuild silently
// drops unsupported child types.

#ifndef MPAPP_HANDLERS_WINDOWS_MENU_BAR_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_MENU_BAR_ITEM_HANDLER_HPP

#include <string>
#include <vector>

#include "../../internal/basic_menu_bar_item.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class menu_bar_item_handler<platform::windows> {
public:
    menu_bar_item_handler();
    ~menu_bar_item_handler();
    menu_bar_item_handler(const menu_bar_item_handler&)            = delete;
    menu_bar_item_handler& operator=(const menu_bar_item_handler&) = delete;

    void map_title(basic_menu_bar_item& m);
    void map_items(basic_menu_bar_item& m);

    winrt::Microsoft::UI::Xaml::Controls::MenuBarItem&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::MenuBarItem& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_menu_bar_item& /*x*/) noexcept {}


private:
    void apply_title(const std::string& v);
    void apply_items(const std::vector<view*>& v);

    struct title_cb_t {
        menu_bar_item_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_title(v); }
    };
    struct items_cb_t {
        menu_bar_item_handler<platform::windows>* self;
        void operator()(const std::vector<view*>& v) const { self->apply_items(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::MenuBarItem native_{nullptr};

    title_cb_t                              title_cb_{this};
    items_cb_t                              items_cb_{this};
    signal_slot<const std::string&>         title_slot_{};
    signal_slot<std::vector<view*> const&>  items_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_MENU_BAR_ITEM_HANDLER_HPP
