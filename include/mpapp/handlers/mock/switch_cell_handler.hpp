// SPDX-License-Identifier: Apache-2.0
// Mock handler for `mpapp::basic_switch_cell`.

#ifndef MPAPP_HANDLERS_MOCK_SWITCH_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_SWITCH_CELL_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../internal/basic_switch_cell.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class switch_cell_handler<platform::mock> : public mock_handler_base {
public:
    switch_cell_handler() = default;
    ~switch_cell_handler() = default;

    switch_cell_handler(const switch_cell_handler&)            = delete;
    switch_cell_handler& operator=(const switch_cell_handler&) = delete;
    switch_cell_handler(switch_cell_handler&&)                 = delete;
    switch_cell_handler& operator=(switch_cell_handler&&)      = delete;

    void map_text(basic_switch_cell& c) {
        record_change("text", c.text.get());
        c.text.changed.subscribe(text_slot_, text_cb_);
    }
    void map_on(basic_switch_cell& c) {
        record_change("on", c.on.get());
        c.on.changed.subscribe(on_slot_, on_cb_);
    }

private:
    using self_t = switch_cell_handler<platform::mock>;
    mock_property_recorder<self_t, std::string> text_cb_{this, "text"};
    mock_property_recorder<self_t, bool>        on_cb_{this, "on"};
    signal_slot<const std::string&>             text_slot_{};
    signal_slot<const bool&>                    on_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_SWITCH_CELL_HANDLER_HPP
