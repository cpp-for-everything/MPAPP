// SPDX-License-Identifier: Apache-2.0
// Mock handler for `mpapp::basic_view_cell`.

#ifndef MPAPP_HANDLERS_MOCK_VIEW_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_VIEW_CELL_HANDLER_HPP

#include "../../platform.hpp"
#include "../../internal/basic_view_cell.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class view_cell_handler<platform::mock> : public mock_handler_base {
public:
    view_cell_handler() = default;
    ~view_cell_handler() = default;

    view_cell_handler(const view_cell_handler&)            = delete;
    view_cell_handler& operator=(const view_cell_handler&) = delete;
    view_cell_handler(view_cell_handler&&)                 = delete;
    view_cell_handler& operator=(view_cell_handler&&)      = delete;

    void map_content(basic_view_cell& c) {
        record_change("content.present", c.content.get() != nullptr);
        c.content.changed.subscribe(slot_, cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_view_cell& /*x*/) noexcept {}


private:
    using self_t = view_cell_handler<platform::mock>;
    struct recorder {
        self_t* self = nullptr;
        void operator()(view* v) const { self->record_change("content.present", v != nullptr); }
    };
    recorder              cb_{this};
    signal_slot<view* const&> slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_VIEW_CELL_HANDLER_HPP
