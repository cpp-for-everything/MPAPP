// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/SearchBar.md
//
// `mpapp::search_bar` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_search_bar` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::search_bar x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_search_bar x;
//     mpapp::search_bar_handler<mpapp::platform::mock> h;
//     h.map_text(x);

#ifndef MPAPP_SEARCH_BAR_HPP
#define MPAPP_SEARCH_BAR_HPP

#include "internal/basic_search_bar.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_search_bar` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/search_bar_handler.hpp"

namespace mpapp {

class search_bar : public internal::basic_search_bar {
public:
    search_bar() {
        set_handler(embedded_handler_);
        embedded_handler_.map_text(*this);
        embedded_handler_.map_placeholder(*this);
        embedded_handler_.map_gestures(*this);
    }

    search_bar(const search_bar&)            = delete;
    search_bar& operator=(const search_bar&) = delete;
    search_bar(search_bar&&)                 = delete;
    search_bar& operator=(search_bar&&)      = delete;

private:
    internal::search_bar_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::search_bar_handler<>` (host-current) and
// `mpapp::search_bar_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using search_bar_handler = internal::search_bar_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_SEARCH_BAR_HPP
