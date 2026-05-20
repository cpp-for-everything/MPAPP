// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock indicator_view handler.

#ifndef MPAPP_HANDLERS_MOCK_INDICATOR_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_INDICATOR_VIEW_HANDLER_HPP

#include "../../indicator_view.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class indicator_view_handler<platform::mock> : public mock_handler_base {
public:
    indicator_view_handler() = default;

    void map_count(indicator_view& iv) {
        bind("count", iv.count, binding_count_);
    }

    void map_position(indicator_view& iv) {
        bind("position", iv.position, binding_position_);
    }

    void map_indicator_color(indicator_view& iv) {
        bind("indicator_color", iv.indicator_color, binding_indicator_color_);
    }

    void map_selected_indicator_color(indicator_view& iv) {
        bind("selected_indicator_color",
             iv.selected_indicator_color,
             binding_selected_indicator_color_);
    }

private:
    detail::property_binding<int>       binding_count_{};
    detail::property_binding<int>       binding_position_{};
    detail::property_binding<brush_ref> binding_indicator_color_{};
    detail::property_binding<brush_ref> binding_selected_indicator_color_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_INDICATOR_VIEW_HANDLER_HPP
