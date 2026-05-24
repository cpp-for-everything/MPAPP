// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/EntryCell.md
//
// `mpapp::entry_cell` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_entry_cell` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::entry_cell x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_entry_cell x;
//     mpapp::entry_cell_handler<mpapp::platform::mock> h;
//     h.map_label(x);

#ifndef MPAPP_ENTRY_CELL_HPP
#define MPAPP_ENTRY_CELL_HPP

#include "internal/basic_entry_cell.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_entry_cell` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/entry_cell_handler.hpp"

namespace mpapp {

class entry_cell : public internal::basic_entry_cell {
public:
    entry_cell() {
        set_ec_handler(embedded_handler_);
        embedded_handler_.map_label(*this);
        embedded_handler_.map_text(*this);
        embedded_handler_.map_placeholder(*this);
        embedded_handler_.map_keyboard(*this);
    }

    entry_cell(const entry_cell&)            = delete;
    entry_cell& operator=(const entry_cell&) = delete;
    entry_cell(entry_cell&&)                 = delete;
    entry_cell& operator=(entry_cell&&)      = delete;

private:
    internal::entry_cell_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::entry_cell_handler<>` (host-current) and
// `mpapp::entry_cell_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using entry_cell_handler = internal::entry_cell_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_ENTRY_CELL_HPP
