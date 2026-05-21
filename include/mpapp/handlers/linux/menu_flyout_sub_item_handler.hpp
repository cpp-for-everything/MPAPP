// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 `menu_flyout_sub_item` handler — wraps a
// `GtkMenuButton` whose `popover` is a nested `GtkPopover` containing
// a vertical `GtkBox` for the submenu children. `text` flows through
// to the button label; `items` rebuild the nested box from the
// ADR-0013 dispatch registry.

#ifndef MPAPP_HANDLERS_LINUX_MENU_FLYOUT_SUB_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_MENU_FLYOUT_SUB_ITEM_HANDLER_HPP

#include <string>
#include <vector>

#include "../../menu_flyout_sub_item.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class menu_flyout_sub_item_handler<platform::linux_> {
public:
    menu_flyout_sub_item_handler();
    ~menu_flyout_sub_item_handler();
    menu_flyout_sub_item_handler(const menu_flyout_sub_item_handler&)            = delete;
    menu_flyout_sub_item_handler& operator=(const menu_flyout_sub_item_handler&) = delete;
    menu_flyout_sub_item_handler(menu_flyout_sub_item_handler&&)                 = delete;
    menu_flyout_sub_item_handler& operator=(menu_flyout_sub_item_handler&&)      = delete;

    void map_text(menu_flyout_sub_item& s);
    void map_items(menu_flyout_sub_item& s);

    void*       native() noexcept       { return native_; }  // GtkMenuButton*
    const void* native() const noexcept { return native_; }

private:
    void apply_text(const std::string& v);
    void apply_items(const std::vector<view*>& v);

    struct text_cb_t  { menu_flyout_sub_item_handler<platform::linux_>* self; void operator()(const std::string& v)        const { self->apply_text(v); } };
    struct items_cb_t { menu_flyout_sub_item_handler<platform::linux_>* self; void operator()(const std::vector<view*>& v) const { self->apply_items(v); } };

    void* native_     = nullptr;  // GtkMenuButton*
    void* sub_popover_ = nullptr; // GtkPopover*
    void* sub_box_     = nullptr; // GtkBox* (vertical)

    text_cb_t                                  text_cb_{this};
    items_cb_t                                 items_cb_{this};
    signal_slot<const std::string&>            text_slot_{};
    signal_slot<std::vector<view*> const&>     items_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_MENU_FLYOUT_SUB_ITEM_HANDLER_HPP
