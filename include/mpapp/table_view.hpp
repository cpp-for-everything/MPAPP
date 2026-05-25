// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/TableView.md
//
// `mpapp::table_view` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_table_view` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::table_view x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_table_view x;
//     mpapp::table_view_handler<mpapp::platform::mock> h;
//     h.map_sections(x);

#ifndef MPAPP_TABLE_VIEW_HPP
#define MPAPP_TABLE_VIEW_HPP

#include "internal/basic_table_view.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_table_view` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/table_view_handler.hpp"

namespace mpapp {

class table_view : public internal::basic_table_view {
public:
    table_view() {
        set_tv_handler(embedded_handler_);
        embedded_handler_.map_sections(*this);
        embedded_handler_.map_typed_sections(*this);
        embedded_handler_.map_row_height(*this);
        embedded_handler_.map_gestures(*this);
    }

    table_view(const table_view&)            = delete;
    table_view& operator=(const table_view&) = delete;
    table_view(table_view&&)                 = delete;
    table_view& operator=(table_view&&)      = delete;

private:
    internal::table_view_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::table_view_handler<>` (host-current) and
// `mpapp::table_view_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using table_view_handler = internal::table_view_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_TABLE_VIEW_HPP
