// SPDX-License-Identifier: Apache-2.0
// Mock handler for `mpapp::basic_text_cell`.

#ifndef MPAPP_HANDLERS_MOCK_TEXT_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_TEXT_CELL_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../internal/basic_text_cell.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class text_cell_handler<platform::mock> : public mock_handler_base {
public:
    text_cell_handler() = default;
    ~text_cell_handler() = default;

    text_cell_handler(const text_cell_handler&)            = delete;
    text_cell_handler& operator=(const text_cell_handler&) = delete;
    text_cell_handler(text_cell_handler&&)                 = delete;
    text_cell_handler& operator=(text_cell_handler&&)      = delete;

    void map_text(basic_text_cell& c) {
        record_change("text", c.text.get());
        c.text.changed.subscribe(text_slot_, text_cb_);
    }
    void map_detail(basic_text_cell& c) {
        record_change("detail", c.detail.get());
        c.detail.changed.subscribe(detail_slot_, detail_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_text_cell& /*x*/) noexcept {}


private:
    using self_t = text_cell_handler<platform::mock>;
    mock_property_recorder<self_t, std::string> text_cb_{this, "text"};
    mock_property_recorder<self_t, std::string> detail_cb_{this, "detail"};
    signal_slot<const std::string&>             text_slot_{};
    signal_slot<const std::string&>             detail_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_TEXT_CELL_HANDLER_HPP
