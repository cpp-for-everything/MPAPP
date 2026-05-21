// SPDX-License-Identifier: Apache-2.0
// Mock handler for `mpapp::graphics_view`.

#ifndef MPAPP_HANDLERS_MOCK_GRAPHICS_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_GRAPHICS_VIEW_HANDLER_HPP

#include <cstddef>

#include "../../graphics_view.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class graphics_view_handler<platform::mock> : public mock_handler_base {
public:
    graphics_view_handler() = default;
    ~graphics_view_handler() = default;

    graphics_view_handler(const graphics_view_handler&)            = delete;
    graphics_view_handler& operator=(const graphics_view_handler&) = delete;
    graphics_view_handler(graphics_view_handler&&)                 = delete;
    graphics_view_handler& operator=(graphics_view_handler&&)      = delete;

    void map_draw_count(graphics_view& gv) {
        record_change("draw_count", gv.draw_count.get());
        gv.draw_count.changed.subscribe(slot_count_, count_cb_);
    }

    void map_size(graphics_view& gv) {
        record_change("width", gv.width.get());
        record_change("height", gv.height.get());
        gv.width.changed.subscribe(slot_w_, w_cb_);
        gv.height.changed.subscribe(slot_h_, h_cb_);
    }

private:
    using self_t = graphics_view_handler<platform::mock>;

    struct count_recorder {
        self_t* self = nullptr;
        void operator()(std::size_t v) const { self->record_change("draw_count", v); }
    };
    struct w_recorder {
        self_t* self = nullptr;
        void operator()(int v) const { self->record_change("width", v); }
    };
    struct h_recorder {
        self_t* self = nullptr;
        void operator()(int v) const { self->record_change("height", v); }
    };

    count_recorder count_cb_{this};
    w_recorder     w_cb_{this};
    h_recorder     h_cb_{this};

    signal_slot<const std::size_t&> slot_count_{};
    signal_slot<const int&>         slot_w_{};
    signal_slot<const int&>         slot_h_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_GRAPHICS_VIEW_HANDLER_HPP
