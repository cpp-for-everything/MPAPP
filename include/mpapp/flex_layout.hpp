// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/FlexLayout.md
//
// `mpapp::flex_layout` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_flex_layout` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::flex_layout x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_flex_layout x;
//     mpapp::flex_layout_handler<mpapp::platform::mock> h;
//     h.map_direction(x);

#ifndef MPAPP_FLEX_LAYOUT_HPP
#define MPAPP_FLEX_LAYOUT_HPP

#include "internal/basic_flex_layout.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_flex_layout` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/flex_layout_handler.hpp"

namespace mpapp {

class flex_layout : public internal::basic_flex_layout {
public:
    flex_layout() {
        set_handler(embedded_handler_);
        embedded_handler_.map_direction(*this);
        embedded_handler_.map_wrap(*this);
        embedded_handler_.map_justify_content(*this);
        embedded_handler_.map_align_items(*this);
        embedded_handler_.map_align_content(*this);
        embedded_handler_.map_position(*this);
        embedded_handler_.map_gestures(*this);
    }

    flex_layout(const flex_layout&)            = delete;
    flex_layout& operator=(const flex_layout&) = delete;
    flex_layout(flex_layout&&)                 = delete;
    flex_layout& operator=(flex_layout&&)      = delete;

private:
    internal::flex_layout_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::flex_layout_handler<>` (host-current) and
// `mpapp::flex_layout_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using flex_layout_handler = internal::flex_layout_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_FLEX_LAYOUT_HPP
