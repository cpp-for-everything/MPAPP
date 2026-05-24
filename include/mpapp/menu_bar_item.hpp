// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MenuBarItem.md
//
// `mpapp::menu_bar_item` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_menu_bar_item` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::menu_bar_item x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_menu_bar_item x;
//     mpapp::menu_bar_item_handler<mpapp::platform::mock> h;
//     h.map_title(x);

#ifndef MPAPP_MENU_BAR_ITEM_HPP
#define MPAPP_MENU_BAR_ITEM_HPP

#include "internal/basic_menu_bar_item.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_menu_bar_item` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/menu_bar_item_handler.hpp"

namespace mpapp {

class menu_bar_item : public internal::basic_menu_bar_item {
public:
    menu_bar_item() {
        set_handler(embedded_handler_);
        embedded_handler_.map_title(*this);
        embedded_handler_.map_items(*this);
    }

    menu_bar_item(const menu_bar_item&)            = delete;
    menu_bar_item& operator=(const menu_bar_item&) = delete;
    menu_bar_item(menu_bar_item&&)                 = delete;
    menu_bar_item& operator=(menu_bar_item&&)      = delete;

private:
    internal::menu_bar_item_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::menu_bar_item_handler<>` (host-current) and
// `mpapp::menu_bar_item_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using menu_bar_item_handler = internal::menu_bar_item_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_MENU_BAR_ITEM_HPP
