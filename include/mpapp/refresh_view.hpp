// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/RefreshView.md
//
// `mpapp::refresh_view` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_refresh_view` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::refresh_view x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_refresh_view x;
//     mpapp::refresh_view_handler<mpapp::platform::mock> h;
//     h.map_content(x);

#ifndef MPAPP_REFRESH_VIEW_HPP
#define MPAPP_REFRESH_VIEW_HPP

#include "internal/basic_refresh_view.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_refresh_view` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/refresh_view_handler.hpp"

namespace mpapp {

class refresh_view : public internal::basic_refresh_view {
public:
    refresh_view() {
        set_handler(embedded_handler_);
        embedded_handler_.map_content(*this);
        embedded_handler_.map_is_refreshing(*this);
        embedded_handler_.map_refresh_color(*this);
    }

    refresh_view(const refresh_view&)            = delete;
    refresh_view& operator=(const refresh_view&) = delete;
    refresh_view(refresh_view&&)                 = delete;
    refresh_view& operator=(refresh_view&&)      = delete;

private:
    internal::refresh_view_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::refresh_view_handler<>` (host-current) and
// `mpapp::refresh_view_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using refresh_view_handler = internal::refresh_view_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_REFRESH_VIEW_HPP
