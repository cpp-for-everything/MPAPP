// SPDX-License-Identifier: Apache-2.0
// Mock basic_carousel_view handler.

#ifndef MPAPP_HANDLERS_MOCK_CAROUSEL_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_CAROUSEL_VIEW_HANDLER_HPP

#include <cstddef>
#include <string>

#include "../../internal/basic_carousel_view.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class carousel_view_handler<platform::mock> : public mock_handler_base {
public:
    carousel_view_handler() = default;

    void map_items_source(basic_carousel_view& c) {
        record("items_source.count", c.items_source.get().size());
        c.items_source.changed.subscribe(items_slot_, items_cb_);
    }
    void map_position(basic_carousel_view& c) { bind("position", c.position, position_binding_); }
    void map_loop(basic_carousel_view& c) { bind("loop", c.loop, loop_binding_); }
    void map_is_swipe_enabled(basic_carousel_view& c) {
        bind("is_swipe_enabled", c.is_swipe_enabled, swipe_binding_);
    }
    void map_peek_count(basic_carousel_view& c) { bind("peek_count", c.peek_count, peek_binding_); }

    // Test helper: simulate a user swipe settling on `index`, recording
    // the resulting PositionChanged.
    void simulate_swipe(basic_carousel_view& c, int index) {
        c.scroll_to(index);
        record("position_changed", c.position.get());
    }

    void map_gestures(basic_carousel_view& /*c*/) noexcept {}

private:
    struct items_cb_t {
        carousel_view_handler* self;
        void operator()(const std::vector<std::string>& v) const {
            self->record("items_source.count", v.size());
        }
    };

    items_cb_t                                       items_cb_{ this };
    signal_slot<const std::vector<std::string>&>     items_slot_{};
    detail::property_binding<int>                    position_binding_{};
    detail::property_binding<bool>                   loop_binding_{};
    detail::property_binding<bool>                   swipe_binding_{};
    detail::property_binding<int>                    peek_binding_{};
};

} // namespace mpapp::internal

#endif // MPAPP_HANDLERS_MOCK_CAROUSEL_VIEW_HANDLER_HPP
