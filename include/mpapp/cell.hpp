// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0021-tableview-cell-types.md
//
// `mpapp::cell` — abstract base for the table-view cell tree. Cells are
// typed row containers (text_cell, entry_cell, switch_cell, view_cell,
// image_cell) used by TableView. Each subclass exposes its own typed
// Observable surface for value bindings.
//
// The base carries only the cross-cutting `is_enabled` toggle and a
// `tapped` signal. Subclasses derive and add their own properties.

#ifndef MPAPP_CELL_HPP
#define MPAPP_CELL_HPP

#include "observable.hpp"
#include "platform.hpp"
#include "signal.hpp"
#include "view.hpp"

namespace mpapp {

class cell : public view {
public:
    cell() = default;
    ~cell() override = default;

    cell(const cell&)            = delete;
    cell& operator=(const cell&) = delete;
    cell(cell&&)                 = delete;
    cell& operator=(cell&&)      = delete;

    Observable<bool> is_enabled{true};
    signal<>         tapped{};
};

} // namespace mpapp

#endif // MPAPP_CELL_HPP
