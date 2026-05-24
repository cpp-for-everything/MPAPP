// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ContentView.md
//
// `mpapp::content_view` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_content_view` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::content_view x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_content_view x;
//     mpapp::content_view_handler<mpapp::platform::mock> h;
//     h.map_content(x);

#ifndef MPAPP_CONTENT_VIEW_HPP
#define MPAPP_CONTENT_VIEW_HPP

#include "internal/basic_content_view.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_content_view` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/content_view_handler.hpp"

namespace mpapp {

class content_view : public internal::basic_content_view {
public:
    content_view() {
        set_handler(embedded_handler_);
        embedded_handler_.map_content(*this);
    }

    content_view(const content_view&)            = delete;
    content_view& operator=(const content_view&) = delete;
    content_view(content_view&&)                 = delete;
    content_view& operator=(content_view&&)      = delete;

private:
    internal::content_view_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::content_view_handler<>` (host-current) and
// `mpapp::content_view_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using content_view_handler = internal::content_view_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_CONTENT_VIEW_HPP
