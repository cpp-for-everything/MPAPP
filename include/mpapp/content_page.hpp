// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ContentPage.md
//
// `mpapp::content_page` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_content_page` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::content_page x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_content_page x;
//     mpapp::content_page_handler<mpapp::platform::mock> h;
//     h.map_title(x);

#ifndef MPAPP_CONTENT_PAGE_HPP
#define MPAPP_CONTENT_PAGE_HPP

#include "internal/basic_content_page.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_content_page` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/content_page_handler.hpp"

namespace mpapp {

class content_page : public internal::basic_content_page {
public:
    content_page() {
        set_handler(embedded_handler_);
        embedded_handler_.map_title(*this);
        embedded_handler_.map_content(*this);
        embedded_handler_.map_padding(*this);
        embedded_handler_.map_gestures(*this);
    }

    content_page(const content_page&)            = delete;
    content_page& operator=(const content_page&) = delete;
    content_page(content_page&&)                 = delete;
    content_page& operator=(content_page&&)      = delete;

private:
    internal::content_page_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::content_page_handler<>` (host-current) and
// `mpapp::content_page_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using content_page_handler = internal::content_page_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_CONTENT_PAGE_HPP
