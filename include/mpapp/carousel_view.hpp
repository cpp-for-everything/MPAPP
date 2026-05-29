// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/CarouselView.md
//
// `mpapp::carousel_view` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_carousel_view` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code reads
// as `mpapp::carousel_view x; x.items_source = {...};` with no separate
// handler variable (ADR-0024 wrapper-component pattern).
//
// Tests stay on the surface (so they don't drag in the per-platform handler
// library):
//
//     mpapp::internal::basic_carousel_view x;
//     mpapp::carousel_view_handler<mpapp::platform::mock> h;
//     h.map_position(x);

#ifndef MPAPP_CAROUSEL_VIEW_HPP
#define MPAPP_CAROUSEL_VIEW_HPP

#include "internal/basic_carousel_view.hpp"

// Pull in the platform-current handler full definition (the umbrella picks
// the right per-platform header). The handler header sees
// `basic_carousel_view` as a complete type, so its inline/out-of-line bodies
// can access surface members.
#include "handlers/carousel_view_handler.hpp"

namespace mpapp {

class carousel_view : public internal::basic_carousel_view {
public:
    carousel_view() {
        set_handler(embedded_handler_);
        embedded_handler_.map_items_source(*this);
        embedded_handler_.map_position(*this);
        embedded_handler_.map_loop(*this);
        embedded_handler_.map_is_swipe_enabled(*this);
        embedded_handler_.map_peek_count(*this);
        embedded_handler_.map_gestures(*this);
    }

    carousel_view(const carousel_view&)            = delete;
    carousel_view& operator=(const carousel_view&) = delete;
    carousel_view(carousel_view&&)                 = delete;
    carousel_view& operator=(carousel_view&&)      = delete;

private:
    internal::carousel_view_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::carousel_view_handler<>` (host-current) and
// `mpapp::carousel_view_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using carousel_view_handler = internal::carousel_view_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_CAROUSEL_VIEW_HPP
