// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/TabbedView.md
//
// `mpapp::tabbed_view` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_tabbed_view` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::tabbed_view x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_tabbed_view x;
//     mpapp::tabbed_view_handler<mpapp::platform::mock> h;
//     h.map_tab_titles(x);

#ifndef MPAPP_TABBED_VIEW_HPP
#define MPAPP_TABBED_VIEW_HPP

#include "internal/basic_tabbed_view.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_tabbed_view` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/tabbed_view_handler.hpp"

namespace mpapp {

class tabbed_view : public internal::basic_tabbed_view {
public:
    tabbed_view() {
        set_handler(embedded_handler_);
        embedded_handler_.map_tab_titles(*this);
        embedded_handler_.map_selected_index(*this);
        embedded_handler_.map_gestures(*this);
    }

    tabbed_view(const tabbed_view&)            = delete;
    tabbed_view& operator=(const tabbed_view&) = delete;
    tabbed_view(tabbed_view&&)                 = delete;
    tabbed_view& operator=(tabbed_view&&)      = delete;

private:
    internal::tabbed_view_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::tabbed_view_handler<>` (host-current) and
// `mpapp::tabbed_view_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using tabbed_view_handler = internal::tabbed_view_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_TABBED_VIEW_HPP
