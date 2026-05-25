// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Grid.md
//
// `grid_layout_handler<platform::mock>` — records the four basic grid
// property mappers (row_count, column_count, row_spacing,
// column_spacing). Per-child cell placement lands with the full track
// definition surface in M-04.

#ifndef MPAPP_HANDLERS_MOCK_GRID_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_GRID_LAYOUT_HANDLER_HPP

#include "../../internal/basic_grid_layout.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class grid_layout_handler<platform::mock> : public mock_handler_base {
public:
    grid_layout_handler() = default;
    ~grid_layout_handler() = default;

    grid_layout_handler(const grid_layout_handler&)            = delete;
    grid_layout_handler& operator=(const grid_layout_handler&) = delete;
    grid_layout_handler(grid_layout_handler&&)                 = delete;
    grid_layout_handler& operator=(grid_layout_handler&&)      = delete;

    void map_row_count(basic_grid_layout& g) {
        record_change("row_count", g.row_count.get());
        g.row_count.changed.subscribe(rows_slot_, rows_cb_);
    }

    void map_column_count(basic_grid_layout& g) {
        record_change("column_count", g.column_count.get());
        g.column_count.changed.subscribe(cols_slot_, cols_cb_);
    }

    void map_row_spacing(basic_grid_layout& g) {
        record_change("row_spacing", g.row_spacing.get());
        g.row_spacing.changed.subscribe(row_sp_slot_, row_sp_cb_);
    }

    void map_column_spacing(basic_grid_layout& g) {
        record_change("column_spacing", g.column_spacing.get());
        g.column_spacing.changed.subscribe(col_sp_slot_, col_sp_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_grid_layout& /*x*/) noexcept {}


private:
    using self_t = grid_layout_handler<platform::mock>;

    mock_property_recorder<self_t, int>    rows_cb_{this, "row_count"};
    signal_slot<const int&>                rows_slot_{};

    mock_property_recorder<self_t, int>    cols_cb_{this, "column_count"};
    signal_slot<const int&>                cols_slot_{};

    mock_property_recorder<self_t, double> row_sp_cb_{this, "row_spacing"};
    signal_slot<const double&>             row_sp_slot_{};

    mock_property_recorder<self_t, double> col_sp_cb_{this, "column_spacing"};
    signal_slot<const double&>             col_sp_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_GRID_LAYOUT_HANDLER_HPP
