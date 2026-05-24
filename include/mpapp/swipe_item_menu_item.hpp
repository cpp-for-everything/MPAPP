// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/SwipeItemMenuItem.md
//
// `mpapp::swipe_item_menu_item` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_swipe_item_menu_item` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::swipe_item_menu_item x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_swipe_item_menu_item x;
//     mpapp::swipe_item_menu_item_handler<mpapp::platform::mock> h;
//     h.map_text(x);

#ifndef MPAPP_SWIPE_ITEM_MENU_ITEM_HPP
#define MPAPP_SWIPE_ITEM_MENU_ITEM_HPP

#include "internal/basic_swipe_item_menu_item.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_swipe_item_menu_item` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/swipe_item_menu_item_handler.hpp"

namespace mpapp {

class swipe_item_menu_item : public internal::basic_swipe_item_menu_item {
public:
    swipe_item_menu_item() {
        set_handler(embedded_handler_);
        embedded_handler_.map_text(*this);
        embedded_handler_.map_icon_uri(*this);
        embedded_handler_.map_invoked(*this);
    }

    swipe_item_menu_item(const swipe_item_menu_item&)            = delete;
    swipe_item_menu_item& operator=(const swipe_item_menu_item&) = delete;
    swipe_item_menu_item(swipe_item_menu_item&&)                 = delete;
    swipe_item_menu_item& operator=(swipe_item_menu_item&&)      = delete;

private:
    internal::swipe_item_menu_item_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::swipe_item_menu_item_handler<>` (host-current) and
// `mpapp::swipe_item_menu_item_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using swipe_item_menu_item_handler = internal::swipe_item_menu_item_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_SWIPE_ITEM_MENU_ITEM_HPP
