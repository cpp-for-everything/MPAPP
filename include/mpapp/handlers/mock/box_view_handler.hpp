// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/BoxView.md
//
// `box_view_handler<platform::mock>` — records property mappers for the
// `box_view` primitive (fill color + per-corner radius).

#ifndef MPAPP_HANDLERS_MOCK_BOX_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_BOX_VIEW_HANDLER_HPP

#include "../../box_view.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class box_view_handler<platform::mock>
    : public mock_handler_base<box_view_handler<platform::mock>, box_view> {
public:
    box_view_handler() = default;

    void map_fill(box_view& b)    { bind("fill",    b.fill,    binding_fill_); }
    void map_corners(box_view& b) { bind("corners", b.corners, binding_corners_); }

private:
    detail::property_binding<color>         binding_fill_{};
    detail::property_binding<corner_radius> binding_corners_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_BOX_VIEW_HANDLER_HPP
