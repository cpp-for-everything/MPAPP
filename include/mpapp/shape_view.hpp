// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ShapeView.md
//
// `mpapp::shape_view` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_shape_view` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::shape_view x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_shape_view x;
//     mpapp::shape_view_handler<mpapp::platform::mock> h;
//     h.map_kind(x);

#ifndef MPAPP_SHAPE_VIEW_HPP
#define MPAPP_SHAPE_VIEW_HPP

#include "internal/basic_shape_view.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_shape_view` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/shape_view_handler.hpp"

namespace mpapp {

class shape_view : public internal::basic_shape_view {
public:
    shape_view() {
        set_sv_handler(embedded_handler_);
        embedded_handler_.map_kind(*this);
        embedded_handler_.map_data(*this);
        embedded_handler_.map_fill(*this);
        embedded_handler_.map_stroke(*this);
        embedded_handler_.map_stroke_thickness(*this);
        embedded_handler_.map_opacity(*this);
    }

    shape_view(const shape_view&)            = delete;
    shape_view& operator=(const shape_view&) = delete;
    shape_view(shape_view&&)                 = delete;
    shape_view& operator=(shape_view&&)      = delete;

private:
    internal::shape_view_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::shape_view_handler<>` (host-current) and
// `mpapp::shape_view_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using shape_view_handler = internal::shape_view_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_SHAPE_VIEW_HPP
