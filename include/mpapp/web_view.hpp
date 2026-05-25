// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/WebView.md
//
// `mpapp::web_view` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_web_view` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::web_view x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_web_view x;
//     mpapp::web_view_handler<mpapp::platform::mock> h;
//     h.map_url(x);

#ifndef MPAPP_WEB_VIEW_HPP
#define MPAPP_WEB_VIEW_HPP

#include "internal/basic_web_view.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_web_view` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/web_view_handler.hpp"

namespace mpapp {

class web_view : public internal::basic_web_view {
public:
    web_view() {
        set_wv_handler(embedded_handler_);
        embedded_handler_.map_url(*this);
        embedded_handler_.map_html(*this);
        embedded_handler_.map_gestures(*this);
    }

    web_view(const web_view&)            = delete;
    web_view& operator=(const web_view&) = delete;
    web_view(web_view&&)                 = delete;
    web_view& operator=(web_view&&)      = delete;

private:
    internal::web_view_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::web_view_handler<>` (host-current) and
// `mpapp::web_view_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using web_view_handler = internal::web_view_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_WEB_VIEW_HPP
