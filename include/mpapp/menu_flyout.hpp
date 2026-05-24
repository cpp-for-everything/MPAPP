// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MenuFlyout.md
//
// `mpapp::menu_flyout` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_menu_flyout` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::menu_flyout x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_menu_flyout x;
//     mpapp::menu_flyout_handler<mpapp::platform::mock> h;
//     h.map_items(x);

#ifndef MPAPP_MENU_FLYOUT_HPP
#define MPAPP_MENU_FLYOUT_HPP

#include "internal/basic_menu_flyout.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_menu_flyout` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/menu_flyout_handler.hpp"

namespace mpapp {

class menu_flyout : public internal::basic_menu_flyout {
public:
    menu_flyout() {
        set_handler(embedded_handler_);
        embedded_handler_.map_items(*this);
        embedded_handler_.map_is_open(*this);
    }

    menu_flyout(const menu_flyout&)            = delete;
    menu_flyout& operator=(const menu_flyout&) = delete;
    menu_flyout(menu_flyout&&)                 = delete;
    menu_flyout& operator=(menu_flyout&&)      = delete;

private:
    internal::menu_flyout_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::menu_flyout_handler<>` (host-current) and
// `mpapp::menu_flyout_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using menu_flyout_handler = internal::menu_flyout_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_MENU_FLYOUT_HPP
