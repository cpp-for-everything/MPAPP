// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/TitleBar.md
//
// `mpapp::title_bar` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_title_bar` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::title_bar x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_title_bar x;
//     mpapp::title_bar_handler<mpapp::platform::mock> h;
//     h.map_title(x);

#ifndef MPAPP_TITLE_BAR_HPP
#define MPAPP_TITLE_BAR_HPP

#include "internal/basic_title_bar.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_title_bar` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/title_bar_handler.hpp"

namespace mpapp {

class title_bar : public internal::basic_title_bar {
public:
    title_bar() {
        set_handler(embedded_handler_);
        embedded_handler_.map_title(*this);
        embedded_handler_.map_subtitle(*this);
        embedded_handler_.map_gestures(*this);
    }

    title_bar(const title_bar&)            = delete;
    title_bar& operator=(const title_bar&) = delete;
    title_bar(title_bar&&)                 = delete;
    title_bar& operator=(title_bar&&)      = delete;

private:
    internal::title_bar_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::title_bar_handler<>` (host-current) and
// `mpapp::title_bar_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using title_bar_handler = internal::title_bar_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_TITLE_BAR_HPP
