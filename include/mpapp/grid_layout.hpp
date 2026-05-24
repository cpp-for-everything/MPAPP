// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/GridLayout.md
//
// `mpapp::grid_layout` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_grid_layout` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::grid_layout x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_grid_layout x;
//     mpapp::grid_layout_handler<mpapp::platform::mock> h;
//     h.map_row_definitions(x);

#ifndef MPAPP_GRID_LAYOUT_HPP
#define MPAPP_GRID_LAYOUT_HPP

#include "internal/basic_grid_layout.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_grid_layout` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/grid_layout_handler.hpp"

namespace mpapp {

class grid_layout : public internal::basic_grid_layout {
public:
    grid_layout() {
        set_handler(embedded_handler_);
        embedded_handler_.map_row_definitions(*this);
        embedded_handler_.map_column_definitions(*this);
        embedded_handler_.map_row_spacing(*this);
        embedded_handler_.map_column_spacing(*this);
    }

    grid_layout(const grid_layout&)            = delete;
    grid_layout& operator=(const grid_layout&) = delete;
    grid_layout(grid_layout&&)                 = delete;
    grid_layout& operator=(grid_layout&&)      = delete;

private:
    internal::grid_layout_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::grid_layout_handler<>` (host-current) and
// `mpapp::grid_layout_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using grid_layout_handler = internal::grid_layout_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_GRID_LAYOUT_HPP
