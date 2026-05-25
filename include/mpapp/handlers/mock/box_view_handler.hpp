// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/BoxView.md
//
// `box_view_handler<platform::mock>` — records property mappers for the
// `basic_box_view` primitive (fill color + per-corner radius).

#ifndef MPAPP_HANDLERS_MOCK_BOX_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_BOX_VIEW_HANDLER_HPP

#include "../../internal/basic_box_view.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class box_view_handler<platform::mock>
    : public mock_handler_base {
public:
    box_view_handler() = default;

    void map_fill(basic_box_view& b)    { bind("fill",    b.fill,    binding_fill_); }
    void map_corners(basic_box_view& b) { bind("corners", b.corners, binding_corners_); }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_box_view& /*x*/) noexcept {}


private:
    detail::property_binding<color>         binding_fill_{};
    detail::property_binding<corner_radius> binding_corners_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_BOX_VIEW_HANDLER_HPP
