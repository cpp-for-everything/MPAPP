// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/GraphicsView.md
//
// `mpapp::graphics_view` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_graphics_view` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::graphics_view x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_graphics_view x;
//     mpapp::graphics_view_handler<mpapp::platform::mock> h;
//     h.map_size(x);

#ifndef MPAPP_GRAPHICS_VIEW_HPP
#define MPAPP_GRAPHICS_VIEW_HPP

#include "internal/basic_graphics_view.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_graphics_view` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/graphics_view_handler.hpp"

namespace mpapp {

class graphics_view : public internal::basic_graphics_view {
public:
    graphics_view() {
        set_gv_handler(embedded_handler_);
        embedded_handler_.map_size(*this);
        embedded_handler_.map_draw_count(*this);
        embedded_handler_.map_drawable(*this);
    }

    graphics_view(const graphics_view&)            = delete;
    graphics_view& operator=(const graphics_view&) = delete;
    graphics_view(graphics_view&&)                 = delete;
    graphics_view& operator=(graphics_view&&)      = delete;

private:
    internal::graphics_view_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::graphics_view_handler<>` (host-current) and
// `mpapp::graphics_view_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using graphics_view_handler = internal::graphics_view_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_GRAPHICS_VIEW_HPP
