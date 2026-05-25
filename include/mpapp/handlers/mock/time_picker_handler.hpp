// SPDX-License-Identifier: Apache-2.0
// Mock basic_time_picker handler.

#ifndef MPAPP_HANDLERS_MOCK_TIME_PICKER_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_TIME_PICKER_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../internal/basic_time_picker.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class time_picker_handler<platform::mock>
    : public mock_handler_base {
public:
    time_picker_handler() = default;

    void map_time(basic_time_picker& p)   { bind("time",   p.time,   binding_time_); }
    void map_format(basic_time_picker& p) { bind("format", p.format, binding_format_); }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_time_picker& /*x*/) noexcept {}


private:
    detail::property_binding<time_value>   binding_time_{};
    detail::property_binding<std::string>  binding_format_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_TIME_PICKER_HANDLER_HPP
