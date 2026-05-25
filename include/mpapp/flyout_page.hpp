// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/FlyoutPage.md
//
// `mpapp::flyout_page` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_flyout_page` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::flyout_page x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_flyout_page x;
//     mpapp::flyout_page_handler<mpapp::platform::mock> h;
//     h.map_flyout(x);

#ifndef MPAPP_FLYOUT_PAGE_HPP
#define MPAPP_FLYOUT_PAGE_HPP

#include "internal/basic_flyout_page.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_flyout_page` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/flyout_page_handler.hpp"

namespace mpapp {

class flyout_page : public internal::basic_flyout_page {
public:
    flyout_page() {
        set_fp_handler(embedded_handler_);
        embedded_handler_.map_flyout(*this);
        embedded_handler_.map_detail(*this);
        embedded_handler_.map_is_presented(*this);
        embedded_handler_.map_gestures(*this);
    }

    flyout_page(const flyout_page&)            = delete;
    flyout_page& operator=(const flyout_page&) = delete;
    flyout_page(flyout_page&&)                 = delete;
    flyout_page& operator=(flyout_page&&)      = delete;

private:
    internal::flyout_page_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::flyout_page_handler<>` (host-current) and
// `mpapp::flyout_page_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using flyout_page_handler = internal::flyout_page_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_FLYOUT_PAGE_HPP
