// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/SwitchCell.md
//
// `mpapp::switch_cell` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_switch_cell` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::switch_cell x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_switch_cell x;
//     mpapp::switch_cell_handler<mpapp::platform::mock> h;
//     h.map_text(x);

#ifndef MPAPP_SWITCH_CELL_HPP
#define MPAPP_SWITCH_CELL_HPP

#include "internal/basic_switch_cell.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_switch_cell` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/switch_cell_handler.hpp"

namespace mpapp {

class switch_cell : public internal::basic_switch_cell {
public:
    switch_cell() {
        set_sc_handler(embedded_handler_);
        embedded_handler_.map_text(*this);
        embedded_handler_.map_on(*this);
        embedded_handler_.map_gestures(*this);
    }

    switch_cell(const switch_cell&)            = delete;
    switch_cell& operator=(const switch_cell&) = delete;
    switch_cell(switch_cell&&)                 = delete;
    switch_cell& operator=(switch_cell&&)      = delete;

private:
    internal::switch_cell_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::switch_cell_handler<>` (host-current) and
// `mpapp::switch_cell_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using switch_cell_handler = internal::switch_cell_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_SWITCH_CELL_HPP
