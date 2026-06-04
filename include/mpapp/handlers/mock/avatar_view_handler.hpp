// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/AvatarView.md
//
// `avatar_view_handler<platform::mock>` — records property mappers for
// the `basic_avatar_view` primitive (initials, image_source,
// corner_radius, background, text_color, shape).

#ifndef MPAPP_HANDLERS_MOCK_AVATAR_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_AVATAR_VIEW_HANDLER_HPP

#include <string>

#include "../../internal/basic_avatar_view.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class avatar_view_handler<platform::mock>
    : public mock_handler_base {
public:
    avatar_view_handler() = default;

    void map_initials(basic_avatar_view& a)      { bind("initials",      a.initials,      binding_initials_); }
    void map_image_source(basic_avatar_view& a)  { bind("image_source",  a.image_source,  binding_image_source_); }
    void map_corner_radius(basic_avatar_view& a) { bind("corner_radius", a.corner_radius, binding_corner_radius_); }
    void map_background(basic_avatar_view& a)    { bind("background",    a.background,    binding_background_); }
    void map_text_color(basic_avatar_view& a)    { bind("text_color",    a.text_color,    binding_text_color_); }
    void map_shape(basic_avatar_view& a)         { bind("shape",         a.shape,         binding_shape_); }

// RFC-0003 stub: per-platform real gesture wire-up is
// pending the platform's real-handler task. No-op today
// so the wrapper ctor's unconditional
// `embedded_handler_.map_gestures(*this);` links.
void map_gestures(basic_avatar_view& /*x*/) noexcept {}

private:
    detail::property_binding<std::string>    binding_initials_{};
    detail::property_binding<std::string>    binding_image_source_{};
    detail::property_binding<double>         binding_corner_radius_{};
    detail::property_binding<brush_ref>      binding_background_{};
    detail::property_binding<brush_ref>      binding_text_color_{};
    detail::property_binding<avatar_shape>   binding_shape_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_AVATAR_VIEW_HANDLER_HPP
