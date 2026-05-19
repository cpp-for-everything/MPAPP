// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock activity_indicator handler.

#ifndef MPAPP_HANDLERS_MOCK_ACTIVITY_INDICATOR_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_ACTIVITY_INDICATOR_HANDLER_HPP

#include "../../activity_indicator.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class activity_indicator_handler<platform::mock>
    : public mock_handler_base {
public:
    activity_indicator_handler() = default;

    void map_is_running(activity_indicator& a) { bind("is_running", a.is_running, binding_is_running_); }
    void map_color(activity_indicator& a)      { bind("color",      a.color,      binding_color_); }

private:
    detail::property_binding<bool>      binding_is_running_{};
    detail::property_binding<brush_ref> binding_color_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_ACTIVITY_INDICATOR_HANDLER_HPP
