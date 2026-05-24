// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0021-tableview-cell-types.md
//
// `mpapp::switch_cell` — TableView row with a `text` label + a native
// toggle switch bound to `on`. Two-way; emits `on_changed` after each
// flip.

#ifndef MPAPP_SWITCH_CELL_HPP
#define MPAPP_SWITCH_CELL_HPP

#include <string>

#include "cell.hpp"
#include "observable.hpp"
#include "platform.hpp"
#include "signal.hpp"

namespace mpapp {

template <class Platform = platform::current>
class switch_cell_handler;

class switch_cell : public cell {
public:
    switch_cell() = default;
    ~switch_cell() override = default;

    switch_cell(const switch_cell&)            = delete;
    switch_cell& operator=(const switch_cell&) = delete;
    switch_cell(switch_cell&&)                 = delete;
    switch_cell& operator=(switch_cell&&)      = delete;

    Observable<std::string> text{""};
    Observable<bool>        on{false};

    signal<bool> on_changed{};

    void toggle() {
        const bool v = !on.get();
        on.set(v);
        on_changed.emit(v);
    }

    switch_cell_handler<platform::current>&       sc_handler() noexcept       { return *sc_handler_; }
    const switch_cell_handler<platform::current>& sc_handler() const noexcept { return *sc_handler_; }
    bool                                          has_sc_handler() const noexcept { return sc_handler_ != nullptr; }
    void                                          set_sc_handler(switch_cell_handler<platform::current>& h) noexcept { sc_handler_ = &h; }

private:
    switch_cell_handler<platform::current>* sc_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_SWITCH_CELL_HPP
