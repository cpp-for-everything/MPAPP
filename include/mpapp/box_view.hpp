// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/BoxView.md
//
// `mpapp::box_view` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_box_view` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::box_view x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_box_view x;
//     mpapp::box_view_handler<mpapp::platform::mock> h;
//     h.map_fill(x);

#ifndef MPAPP_BOX_VIEW_HPP
#define MPAPP_BOX_VIEW_HPP

#include "internal/basic_box_view.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_box_view` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/box_view_handler.hpp"

namespace mpapp {

class box_view : public internal::basic_box_view {
public:
    box_view() {
        set_handler(embedded_handler_);
        embedded_handler_.map_fill(*this);
        embedded_handler_.map_corners(*this);
    }

    box_view(const box_view&)            = delete;
    box_view& operator=(const box_view&) = delete;
    box_view(box_view&&)                 = delete;
    box_view& operator=(box_view&&)      = delete;

private:
    internal::box_view_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::box_view_handler<>` (host-current) and
// `mpapp::box_view_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using box_view_handler = internal::box_view_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_BOX_VIEW_HPP
