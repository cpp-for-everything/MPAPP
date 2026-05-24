// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/HybridWebView.md
//
// `mpapp::hybrid_web_view` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_hybrid_web_view` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::hybrid_web_view x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_hybrid_web_view x;
//     mpapp::hybrid_web_view_handler<mpapp::platform::mock> h;
//     h.map_messages(x);

#ifndef MPAPP_HYBRID_WEB_VIEW_HPP
#define MPAPP_HYBRID_WEB_VIEW_HPP

#include "internal/basic_hybrid_web_view.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_hybrid_web_view` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/hybrid_web_view_handler.hpp"

namespace mpapp {

class hybrid_web_view : public internal::basic_hybrid_web_view {
public:
    hybrid_web_view() {
        set_hwv_handler(embedded_handler_);
        embedded_handler_.map_messages(*this);
        embedded_handler_.map_html_source(*this);
    }

    hybrid_web_view(const hybrid_web_view&)            = delete;
    hybrid_web_view& operator=(const hybrid_web_view&) = delete;
    hybrid_web_view(hybrid_web_view&&)                 = delete;
    hybrid_web_view& operator=(hybrid_web_view&&)      = delete;

private:
    internal::hybrid_web_view_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::hybrid_web_view_handler<>` (host-current) and
// `mpapp::hybrid_web_view_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using hybrid_web_view_handler = internal::hybrid_web_view_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_HYBRID_WEB_VIEW_HPP
