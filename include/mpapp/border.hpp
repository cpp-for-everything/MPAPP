// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Border.md
//
// `mpapp::border` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_border` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::border x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_border x;
//     mpapp::border_handler<mpapp::platform::mock> h;
//     h.map_content(x);

#ifndef MPAPP_BORDER_HPP
#define MPAPP_BORDER_HPP

#include "internal/basic_border.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_border` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/border_handler.hpp"

namespace mpapp {

class border : public internal::basic_border {
public:
    border() {
        set_handler(embedded_handler_);
        embedded_handler_.map_content(*this);
        embedded_handler_.map_padding(*this);
        embedded_handler_.map_stroke(*this);
        embedded_handler_.map_stroke_thickness(*this);
        embedded_handler_.map_stroke_shape(*this);
        embedded_handler_.map_gestures(*this);
    }

    border(const border&)            = delete;
    border& operator=(const border&) = delete;
    border(border&&)                 = delete;
    border& operator=(border&&)      = delete;

private:
    internal::border_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::border_handler<>` (host-current) and
// `mpapp::border_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using border_handler = internal::border_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_BORDER_HPP
