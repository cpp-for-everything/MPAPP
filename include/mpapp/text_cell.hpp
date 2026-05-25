// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/TextCell.md
//
// `mpapp::text_cell` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_text_cell` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::text_cell x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_text_cell x;
//     mpapp::text_cell_handler<mpapp::platform::mock> h;
//     h.map_text(x);

#ifndef MPAPP_TEXT_CELL_HPP
#define MPAPP_TEXT_CELL_HPP

#include "internal/basic_text_cell.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_text_cell` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/text_cell_handler.hpp"

namespace mpapp {

class text_cell : public internal::basic_text_cell {
public:
    text_cell() {
        set_tc_handler(embedded_handler_);
        embedded_handler_.map_text(*this);
        embedded_handler_.map_detail(*this);
        embedded_handler_.map_gestures(*this);
    }

    text_cell(const text_cell&)            = delete;
    text_cell& operator=(const text_cell&) = delete;
    text_cell(text_cell&&)                 = delete;
    text_cell& operator=(text_cell&&)      = delete;

private:
    internal::text_cell_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::text_cell_handler<>` (host-current) and
// `mpapp::text_cell_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using text_cell_handler = internal::text_cell_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_TEXT_CELL_HPP
