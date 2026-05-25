// SPDX-License-Identifier: Apache-2.0
// Mock handler for `mpapp::basic_entry_cell`.

#ifndef MPAPP_HANDLERS_MOCK_ENTRY_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_ENTRY_CELL_HANDLER_HPP

#include <string>

#include "../../internal/basic_entry_cell.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class entry_cell_handler<platform::mock> : public mock_handler_base {
public:
    entry_cell_handler() = default;
    ~entry_cell_handler() = default;

    entry_cell_handler(const entry_cell_handler&)            = delete;
    entry_cell_handler& operator=(const entry_cell_handler&) = delete;
    entry_cell_handler(entry_cell_handler&&)                 = delete;
    entry_cell_handler& operator=(entry_cell_handler&&)      = delete;

    void map_label(basic_entry_cell& c) {
        record_change("label", c.label.get());
        c.label.changed.subscribe(label_slot_, label_cb_);
    }
    void map_text(basic_entry_cell& c) {
        record_change("text", c.text.get());
        c.text.changed.subscribe(text_slot_, text_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_entry_cell& /*x*/) noexcept {}


private:
    using self_t = entry_cell_handler<platform::mock>;
    mock_property_recorder<self_t, std::string> label_cb_{this, "label"};
    mock_property_recorder<self_t, std::string> text_cb_{this, "text"};
    signal_slot<const std::string&>             label_slot_{};
    signal_slot<const std::string&>             text_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_ENTRY_CELL_HANDLER_HPP
