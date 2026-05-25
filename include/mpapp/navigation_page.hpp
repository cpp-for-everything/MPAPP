// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/NavigationPage.md
//
// `mpapp::navigation_page` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_navigation_page` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::navigation_page x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_navigation_page x;
//     mpapp::navigation_page_handler<mpapp::platform::mock> h;
//     h.map_stack(x);

#ifndef MPAPP_NAVIGATION_PAGE_HPP
#define MPAPP_NAVIGATION_PAGE_HPP

#include "internal/basic_navigation_page.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_navigation_page` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/navigation_page_handler.hpp"

namespace mpapp {

class navigation_page : public internal::basic_navigation_page {
public:
    navigation_page() {
        set_np_handler(embedded_handler_);
        embedded_handler_.map_stack(*this);
        embedded_handler_.map_gestures(*this);
    }

    navigation_page(const navigation_page&)            = delete;
    navigation_page& operator=(const navigation_page&) = delete;
    navigation_page(navigation_page&&)                 = delete;
    navigation_page& operator=(navigation_page&&)      = delete;

private:
    internal::navigation_page_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::navigation_page_handler<>` (host-current) and
// `mpapp::navigation_page_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using navigation_page_handler = internal::navigation_page_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_NAVIGATION_PAGE_HPP
