// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0021-tableview-cell-types.md
//
// `mpapp::view_cell` — escape hatch. TableView row hosting an arbitrary
// `view*` so apps can ship custom rows the typed cells don't cover.

#ifndef MPAPP_VIEW_CELL_HPP
#define MPAPP_VIEW_CELL_HPP

#include "cell.hpp"
#include "observable.hpp"
#include "platform.hpp"

namespace mpapp {

template <class Platform = platform::current>
class view_cell_handler;

class view_cell : public cell {
public:
    view_cell() = default;
    ~view_cell() override = default;

    view_cell(const view_cell&)            = delete;
    view_cell& operator=(const view_cell&) = delete;
    view_cell(view_cell&&)                 = delete;
    view_cell& operator=(view_cell&&)      = delete;

    Observable<view*> content{nullptr};

    view_cell_handler<platform::current>&       vc_handler() noexcept       { return *vc_handler_; }
    const view_cell_handler<platform::current>& vc_handler() const noexcept { return *vc_handler_; }
    bool                                        has_vc_handler() const noexcept { return vc_handler_ != nullptr; }
    void                                        set_vc_handler(view_cell_handler<platform::current>& h) noexcept { vc_handler_ = &h; }

private:
    view_cell_handler<platform::current>* vc_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_VIEW_CELL_HPP
