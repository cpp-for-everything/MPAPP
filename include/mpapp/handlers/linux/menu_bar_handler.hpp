// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_menu_bar handler — renders a horizontal `GtkBox`
// of buttons, one per child `basic_menu_bar_item`.
//
// `GtkPopoverMenuBar` is the closer-to-MAUI native control but its
// `GMenuModel`-only binding shape is heavyweight for the M-04b
// baseline. Using a flat `GtkBox` of buttons keeps the handler
// self-contained (the buttons reuse the child `basic_menu_bar_item`'s native
// `GtkMenuButton`, so their drop-down popover behavior still works on
// real platforms once basic_menu_flyout lands). Swap-in to PopoverMenuBar
// can happen alongside the M-04c basic_menu_flyout work.

#ifndef MPAPP_HANDLERS_LINUX_MENU_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_MENU_BAR_HANDLER_HPP

#include <vector>

#include "../../internal/basic_menu_bar.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class menu_bar_handler<platform::linux_> {
public:
    menu_bar_handler();
    ~menu_bar_handler();
    menu_bar_handler(const menu_bar_handler&)            = delete;
    menu_bar_handler& operator=(const menu_bar_handler&) = delete;

    void map_items(basic_menu_bar& b);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_menu_bar& x);


private:
    void apply_items(const std::vector<view*>& v);

    struct items_cb_t {
        menu_bar_handler<platform::linux_>* self;
        void operator()(const std::vector<view*>& v) const { self->apply_items(v); }
    };

    void* native_ = nullptr;  // GtkBox* (horizontal)

    items_cb_t                              items_cb_{this};
    signal_slot<std::vector<view*> const&>  items_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_MENU_BAR_HANDLER_HPP
