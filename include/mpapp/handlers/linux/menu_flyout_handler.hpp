// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 `menu_flyout` handler — wraps a `GtkPopover`
// containing a vertical `GtkBox`. Each child resolved via the
// ADR-0013 dispatch registry is appended into the box; the popover
// itself is the `native()` exposed to dispatch sites (but it returns
// nullptr from its dispatcher because a popover is a top-level
// surface, not a child of a regular container). `is_open` calls
// `gtk_popover_popup` / `gtk_popover_popdown` on the native widget.
//
// `GtkPopoverMenu` would be the more idiomatic choice but it requires
// a `GMenuModel` action-backed structure that doesn't map cleanly to
// the cross-platform `items: vector<view*>` surface. The popover-with-
// box approach lets each menu_flyout_item / _separator / _sub_item
// stay a regular widget that registers with the dispatch registry.

#ifndef MPAPP_HANDLERS_LINUX_MENU_FLYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_MENU_FLYOUT_HANDLER_HPP

#include <vector>

#include "../../menu_flyout.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class menu_flyout_handler<platform::linux_> {
public:
    menu_flyout_handler();
    ~menu_flyout_handler();
    menu_flyout_handler(const menu_flyout_handler&)            = delete;
    menu_flyout_handler& operator=(const menu_flyout_handler&) = delete;
    menu_flyout_handler(menu_flyout_handler&&)                 = delete;
    menu_flyout_handler& operator=(menu_flyout_handler&&)      = delete;

    void map_items(menu_flyout& f);
    void map_is_open(menu_flyout& f);

    void*       native() noexcept       { return native_; }   // GtkPopover*
    const void* native() const noexcept { return native_; }

private:
    void apply_items(const std::vector<view*>& v);
    void apply_is_open(bool v);

    struct items_cb_t   { menu_flyout_handler<platform::linux_>* self; void operator()(const std::vector<view*>& v) const { self->apply_items(v); } };
    struct is_open_cb_t { menu_flyout_handler<platform::linux_>* self; void operator()(bool v)                      const { self->apply_is_open(v); } };

    void* native_ = nullptr;  // GtkPopover*
    void* box_    = nullptr;  // GtkBox*    — vertical children host

    items_cb_t                                items_cb_{this};
    is_open_cb_t                              is_open_cb_{this};
    signal_slot<std::vector<view*> const&>    items_slot_{};
    signal_slot<const bool&>                  is_open_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_MENU_FLYOUT_HANDLER_HPP
