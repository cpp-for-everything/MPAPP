// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ActivityIndicator.md
//
// `mpapp::activity_indicator` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_activity_indicator` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::activity_indicator x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_activity_indicator x;
//     mpapp::activity_indicator_handler<mpapp::platform::mock> h;
//     h.map_is_running(x);

#ifndef MPAPP_ACTIVITY_INDICATOR_HPP
#define MPAPP_ACTIVITY_INDICATOR_HPP

#include "internal/basic_activity_indicator.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_activity_indicator` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/activity_indicator_handler.hpp"

namespace mpapp {

class activity_indicator : public internal::basic_activity_indicator {
public:
    activity_indicator() {
        set_handler(embedded_handler_);
        embedded_handler_.map_is_running(*this);
        embedded_handler_.map_color(*this);
    }

    activity_indicator(const activity_indicator&)            = delete;
    activity_indicator& operator=(const activity_indicator&) = delete;
    activity_indicator(activity_indicator&&)                 = delete;
    activity_indicator& operator=(activity_indicator&&)      = delete;

private:
    internal::activity_indicator_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::activity_indicator_handler<>` (host-current) and
// `mpapp::activity_indicator_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using activity_indicator_handler = internal::activity_indicator_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_ACTIVITY_INDICATOR_HPP
