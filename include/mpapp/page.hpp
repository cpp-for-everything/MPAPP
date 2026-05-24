// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Page.md
//
// `mpapp::page` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_page` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::page x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_page x;
//     mpapp::page_handler<mpapp::platform::mock> h;
//     h.map_title(x);

#ifndef MPAPP_PAGE_HPP
#define MPAPP_PAGE_HPP

#include "internal/basic_page.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_page` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/page_handler.hpp"

namespace mpapp {

class page : public internal::basic_page {
public:
    page() {
        set_handler(embedded_handler_);
        embedded_handler_.map_title(*this);
        embedded_handler_.map_content(*this);
        embedded_handler_.map_is_busy(*this);
    }

    page(const page&)            = delete;
    page& operator=(const page&) = delete;
    page(page&&)                 = delete;
    page& operator=(page&&)      = delete;

private:
    internal::page_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::page_handler<>` (host-current) and
// `mpapp::page_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using page_handler = internal::page_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_PAGE_HPP
