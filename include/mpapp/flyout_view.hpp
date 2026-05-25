// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/FlyoutView.md
//
// `mpapp::flyout_view` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_flyout_view` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::flyout_view x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_flyout_view x;
//     mpapp::flyout_view_handler<mpapp::platform::mock> h;
//     h.map_flyout(x);

#ifndef MPAPP_FLYOUT_VIEW_HPP
#define MPAPP_FLYOUT_VIEW_HPP

#include "internal/basic_flyout_view.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_flyout_view` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/flyout_view_handler.hpp"

namespace mpapp {

class flyout_view : public internal::basic_flyout_view {
public:
    flyout_view() {
        set_handler(embedded_handler_);
        embedded_handler_.map_flyout(*this);
        embedded_handler_.map_detail(*this);
        embedded_handler_.map_is_presented(*this);
        embedded_handler_.map_gestures(*this);
    }

    flyout_view(const flyout_view&)            = delete;
    flyout_view& operator=(const flyout_view&) = delete;
    flyout_view(flyout_view&&)                 = delete;
    flyout_view& operator=(flyout_view&&)      = delete;

private:
    internal::flyout_view_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::flyout_view_handler<>` (host-current) and
// `mpapp::flyout_view_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using flyout_view_handler = internal::flyout_view_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_FLYOUT_VIEW_HPP
