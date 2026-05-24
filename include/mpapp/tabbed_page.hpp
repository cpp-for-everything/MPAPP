// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/TabbedPage.md
//
// `mpapp::tabbed_page` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_tabbed_page` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::tabbed_page x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_tabbed_page x;
//     mpapp::tabbed_page_handler<mpapp::platform::mock> h;
//     h.map_children(x);

#ifndef MPAPP_TABBED_PAGE_HPP
#define MPAPP_TABBED_PAGE_HPP

#include "internal/basic_tabbed_page.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_tabbed_page` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/tabbed_page_handler.hpp"

namespace mpapp {

class tabbed_page : public internal::basic_tabbed_page {
public:
    tabbed_page() {
        set_tp_handler(embedded_handler_);
        embedded_handler_.map_children(*this);
        embedded_handler_.map_selected_index(*this);
    }

    tabbed_page(const tabbed_page&)            = delete;
    tabbed_page& operator=(const tabbed_page&) = delete;
    tabbed_page(tabbed_page&&)                 = delete;
    tabbed_page& operator=(tabbed_page&&)      = delete;

private:
    internal::tabbed_page_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::tabbed_page_handler<>` (host-current) and
// `mpapp::tabbed_page_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using tabbed_page_handler = internal::tabbed_page_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_TABBED_PAGE_HPP
