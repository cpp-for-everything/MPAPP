// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock basic_progress_bar handler.

#ifndef MPAPP_HANDLERS_MOCK_PROGRESS_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_PROGRESS_BAR_HANDLER_HPP

#include "../../platform.hpp"
#include "../../internal/basic_progress_bar.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class progress_bar_handler<platform::mock>
    : public mock_handler_base {
public:
    progress_bar_handler() = default;

    void map_progress(basic_progress_bar& p)         { bind("progress",         p.progress,         binding_progress_); }
    void map_color(basic_progress_bar& p)            { bind("color",            p.color,            binding_color_); }
    void map_background_color(basic_progress_bar& p) { bind("background_color", p.background_color, binding_bg_); }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_progress_bar& /*x*/) noexcept {}


private:
    detail::property_binding<double>    binding_progress_{};
    detail::property_binding<brush_ref> binding_color_{};
    detail::property_binding<brush_ref> binding_bg_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_PROGRESS_BAR_HANDLER_HPP
