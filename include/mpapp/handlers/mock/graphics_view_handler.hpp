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

    // Records whether the user has installed a non-null draw callback
    // (1) or cleared it (0). Tests can assert that the mock observed
    // the install + clear lifecycle.
    void map_drawable(graphics_view& gv) {
        record_change("drawable", gv.drawable.get() ? 1 : 0);
        last_drawable_set = static_cast<bool>(gv.drawable.get());
        gv.drawable.changed.subscribe(slot_drawable_, drawable_cb_);
    }

    // Most-recent install state of the drawable callback. False until
    // map_drawable has run and a non-null function has been set.
    bool last_drawable_set = false;

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
    struct drawable_recorder {
        self_t* self = nullptr;
        void operator()(const graphics_view::draw_callback_t& f) const {
            self->last_drawable_set = static_cast<bool>(f);
            self->record_change("drawable", f ? 1 : 0);
        }
    };

    count_recorder    count_cb_{this};
    w_recorder        w_cb_{this};
    h_recorder        h_cb_{this};
    drawable_recorder drawable_cb_{this};

    signal_slot<const std::size_t&>                    slot_count_{};
    signal_slot<const int&>                            slot_w_{};
    signal_slot<const int&>                            slot_h_{};
    signal_slot<const graphics_view::draw_callback_t&> slot_drawable_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_GRAPHICS_VIEW_HANDLER_HPP
