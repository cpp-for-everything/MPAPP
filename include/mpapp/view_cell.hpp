// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ViewCell.md
//
// `mpapp::view_cell` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_view_cell` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::view_cell x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_view_cell x;
//     mpapp::view_cell_handler<mpapp::platform::mock> h;
//     h.map_content(x);

#ifndef MPAPP_VIEW_CELL_HPP
#define MPAPP_VIEW_CELL_HPP

#include "internal/basic_view_cell.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_view_cell` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/view_cell_handler.hpp"

namespace mpapp {

class view_cell : public internal::basic_view_cell {
public:
    view_cell() {
        set_vc_handler(embedded_handler_);
        embedded_handler_.map_content(*this);
        embedded_handler_.map_gestures(*this);
    }

    view_cell(const view_cell&)            = delete;
    view_cell& operator=(const view_cell&) = delete;
    view_cell(view_cell&&)                 = delete;
    view_cell& operator=(view_cell&&)      = delete;

private:
    internal::view_cell_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::view_cell_handler<>` (host-current) and
// `mpapp::view_cell_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using view_cell_handler = internal::view_cell_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_VIEW_CELL_HPP
