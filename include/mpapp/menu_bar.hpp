// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MenuBar.md
//
// `mpapp::menu_bar` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_menu_bar` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::menu_bar x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_menu_bar x;
//     mpapp::menu_bar_handler<mpapp::platform::mock> h;
//     h.map_items(x);

#ifndef MPAPP_MENU_BAR_HPP
#define MPAPP_MENU_BAR_HPP

#include "internal/basic_menu_bar.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_menu_bar` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/menu_bar_handler.hpp"

namespace mpapp {

class menu_bar : public internal::basic_menu_bar {
public:
    menu_bar() {
        set_handler(embedded_handler_);
        embedded_handler_.map_items(*this);
        embedded_handler_.map_gestures(*this);
    }

    menu_bar(const menu_bar&)            = delete;
    menu_bar& operator=(const menu_bar&) = delete;
    menu_bar(menu_bar&&)                 = delete;
    menu_bar& operator=(menu_bar&&)      = delete;

private:
    internal::menu_bar_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::menu_bar_handler<>` (host-current) and
// `mpapp::menu_bar_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using menu_bar_handler = internal::menu_bar_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_MENU_BAR_HPP
