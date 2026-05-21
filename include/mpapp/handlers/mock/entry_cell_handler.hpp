// SPDX-License-Identifier: Apache-2.0
// Mock handler for `mpapp::entry_cell`.

#ifndef MPAPP_HANDLERS_MOCK_ENTRY_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_ENTRY_CELL_HANDLER_HPP

#include <string>

#include "../../entry_cell.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class entry_cell_handler<platform::mock> : public mock_handler_base {
public:
    entry_cell_handler() = default;
    ~entry_cell_handler() = default;

    entry_cell_handler(const entry_cell_handler&)            = delete;
    entry_cell_handler& operator=(const entry_cell_handler&) = delete;
    entry_cell_handler(entry_cell_handler&&)                 = delete;
    entry_cell_handler& operator=(entry_cell_handler&&)      = delete;

    void map_label(entry_cell& c) {
        record_change("label", c.label.get());
        c.label.changed.subscribe(label_slot_, label_cb_);
    }
    void map_text(entry_cell& c) {
        record_change("text", c.text.get());
        c.text.changed.subscribe(text_slot_, text_cb_);
    }

private:
    using self_t = entry_cell_handler<platform::mock>;
    mock_property_recorder<self_t, std::string> label_cb_{this, "label"};
    mock_property_recorder<self_t, std::string> text_cb_{this, "text"};
    signal_slot<const std::string&>             label_slot_{};
    signal_slot<const std::string&>             text_slot_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_ENTRY_CELL_HANDLER_HPP
