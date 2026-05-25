// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/IndicatorView.md
//
// `mpapp::indicator_view` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_indicator_view` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::indicator_view x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_indicator_view x;
//     mpapp::indicator_view_handler<mpapp::platform::mock> h;
//     h.map_count(x);

#ifndef MPAPP_INDICATOR_VIEW_HPP
#define MPAPP_INDICATOR_VIEW_HPP

#include "internal/basic_indicator_view.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_indicator_view` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/indicator_view_handler.hpp"

namespace mpapp {

class indicator_view : public internal::basic_indicator_view {
public:
    indicator_view() {
        set_handler(embedded_handler_);
        embedded_handler_.map_count(*this);
        embedded_handler_.map_position(*this);
        embedded_handler_.map_indicator_color(*this);
        embedded_handler_.map_selected_indicator_color(*this);
        embedded_handler_.map_gestures(*this);
    }

    indicator_view(const indicator_view&)            = delete;
    indicator_view& operator=(const indicator_view&) = delete;
    indicator_view(indicator_view&&)                 = delete;
    indicator_view& operator=(indicator_view&&)      = delete;

private:
    internal::indicator_view_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::indicator_view_handler<>` (host-current) and
// `mpapp::indicator_view_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using indicator_view_handler = internal::indicator_view_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_INDICATOR_VIEW_HPP
