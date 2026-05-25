// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Frame.md
//
// `mpapp::frame` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_frame` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::frame x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_frame x;
//     mpapp::frame_handler<mpapp::platform::mock> h;
//     h.map_content(x);

#ifndef MPAPP_FRAME_HPP
#define MPAPP_FRAME_HPP

#include "internal/basic_frame.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_frame` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/frame_handler.hpp"

namespace mpapp {

class frame : public internal::basic_frame {
public:
    frame() {
        set_handler(embedded_handler_);
        embedded_handler_.map_content(*this);
        embedded_handler_.map_border_color(*this);
        embedded_handler_.map_has_shadow(*this);
        embedded_handler_.map_corner_radius(*this);
        embedded_handler_.map_padding(*this);
        embedded_handler_.map_gestures(*this);
    }

    frame(const frame&)            = delete;
    frame& operator=(const frame&) = delete;
    frame(frame&&)                 = delete;
    frame& operator=(frame&&)      = delete;

private:
    internal::frame_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::frame_handler<>` (host-current) and
// `mpapp::frame_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using frame_handler = internal::frame_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_FRAME_HPP
