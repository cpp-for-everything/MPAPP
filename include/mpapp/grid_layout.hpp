// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Grid.md
//
// `mpapp::grid_layout` — 2D layout container with row + column tracks
// and per-child placement. Mirrors MAUI's `Grid`, WinUI's `Grid`,
// GTK4's `GtkGrid`, AppKit's `NSGridView`, UIKit's
// `UIStackView`-of-`UIStackView`s composition.
//
// Mock surface only ships the basics needed by the spike rewrite —
// fixed-count `row_count` / `column_count` and per-child `(row, column)`
// placement via the inherited `update_z_index`-style command pattern.
// Full track definitions (star sizing, min/max constraints) land with
// the M-04 layout work.

#ifndef MPAPP_GRID_LAYOUT_HPP
#define MPAPP_GRID_LAYOUT_HPP

#include "layout.hpp"
#include "observable.hpp"
#include "platform.hpp"

namespace mpapp {

template <class Platform>
class grid_layout_handler;

class grid_layout : public layout {
public:
    grid_layout() = default;
    ~grid_layout() override = default;

    grid_layout(const grid_layout&)            = delete;
    grid_layout& operator=(const grid_layout&) = delete;
    grid_layout(grid_layout&&)                 = delete;
    grid_layout& operator=(grid_layout&&)      = delete;

    Observable<int>    row_count{1};
    Observable<int>    column_count{1};
    Observable<double> row_spacing{0.0};
    Observable<double> column_spacing{0.0};

    grid_layout_handler<platform::current>&       handler() noexcept       { return *grid_handler_; }
    const grid_layout_handler<platform::current>& handler() const noexcept { return *grid_handler_; }
    bool                                          has_handler() const noexcept { return grid_handler_ != nullptr; }
    void                                          set_handler(grid_layout_handler<platform::current>& h) noexcept { grid_handler_ = &h; }

private:
    grid_layout_handler<platform::current>* grid_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_GRID_LAYOUT_HPP
