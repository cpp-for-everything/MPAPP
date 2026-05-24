// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MenuFlyoutSeparator.md
//
// `mpapp::menu_flyout_separator` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_menu_flyout_separator` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::menu_flyout_separator x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_menu_flyout_separator x;
//     mpapp::menu_flyout_separator_handler<mpapp::platform::mock> h;
//     h.map_text(x);

#ifndef MPAPP_MENU_FLYOUT_SEPARATOR_HPP
#define MPAPP_MENU_FLYOUT_SEPARATOR_HPP

#include "internal/basic_menu_flyout_separator.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_menu_flyout_separator` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/menu_flyout_separator_handler.hpp"

namespace mpapp {

class menu_flyout_separator : public internal::basic_menu_flyout_separator {
public:
    menu_flyout_separator() {
        set_handler(embedded_handler_);
    }

    menu_flyout_separator(const menu_flyout_separator&)            = delete;
    menu_flyout_separator& operator=(const menu_flyout_separator&) = delete;
    menu_flyout_separator(menu_flyout_separator&&)                 = delete;
    menu_flyout_separator& operator=(menu_flyout_separator&&)      = delete;

private:
    internal::menu_flyout_separator_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::menu_flyout_separator_handler<>` (host-current) and
// `mpapp::menu_flyout_separator_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using menu_flyout_separator_handler = internal::menu_flyout_separator_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_MENU_FLYOUT_SEPARATOR_HPP
