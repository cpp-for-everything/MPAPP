// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 menu_bar_item handler — wraps a `GtkMenuButton`
// whose label is the entry's `title`. A child popover can be attached
// later (when menu_flyout lands in M-04c) by calling
// `gtk_menu_button_set_popover()` on the same native pointer.
//
// For the M-04b baseline `items` simply tracks the count via the
// handler's own state — granular GtkPopover wiring is deferred until
// the menu_flyout family arrives.

#ifndef MPAPP_HANDLERS_LINUX_MENU_BAR_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_MENU_BAR_ITEM_HANDLER_HPP

#include <string>
#include <vector>

#include "../../menu_bar_item.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class menu_bar_item_handler<platform::linux_> {
public:
    menu_bar_item_handler();
    ~menu_bar_item_handler();
    menu_bar_item_handler(const menu_bar_item_handler&)            = delete;
    menu_bar_item_handler& operator=(const menu_bar_item_handler&) = delete;

    void map_title(menu_bar_item& m);
    void map_items(menu_bar_item& m);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_title(const std::string& v);
    void apply_items(const std::vector<view*>& v);

    struct title_cb_t {
        menu_bar_item_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_title(v); }
    };
    struct items_cb_t {
        menu_bar_item_handler<platform::linux_>* self;
        void operator()(const std::vector<view*>& v) const { self->apply_items(v); }
    };

    void* native_ = nullptr;  // GtkMenuButton*

    title_cb_t                              title_cb_{this};
    items_cb_t                              items_cb_{this};
    signal_slot<const std::string&>         title_slot_{};
    signal_slot<std::vector<view*> const&>  items_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_MENU_BAR_ITEM_HANDLER_HPP
