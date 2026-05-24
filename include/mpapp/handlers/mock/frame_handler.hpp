// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Frame.md
//
// `frame_handler<platform::mock>` — records property mappers for the
// **deprecated** `basic_frame` control. Kept for XAML migration parity.
//
// `mpapp::basic_frame` carries `[[deprecated]]`, so naming it inside this
// header would normally raise a diagnostic at the include site. We
// suppress it locally — the handler IS the legacy migration path, so
// its own use of `basic_frame` is intentional. Internal compiler pragmas are
// not part of the public API surface (CLAUDE Rule 1 covers user-facing
// `MPAPP_*` macros only).

#ifndef MPAPP_HANDLERS_MOCK_FRAME_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_FRAME_HANDLER_HPP

#include "../../internal/basic_frame.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#endif

namespace mpapp::internal {

template <>
class frame_handler<platform::mock>
    : public mock_handler_base {
public:
    frame_handler() = default;

    void map_border_color(basic_frame& f)   { bind("border_color",   f.border_color,   binding_border_color_); }
    void map_has_shadow(basic_frame& f)     { bind("has_shadow",     f.has_shadow,     binding_has_shadow_); }
    void map_corner_radius(basic_frame& f)  { bind("corner_radius",  f.corner_radius,  binding_corner_radius_); }
    void map_padding(basic_frame& f)        { bind("padding",        f.padding,        binding_padding_); }

    void map_content(basic_frame& f) {
        record("content.present", f.content.get() != nullptr);
        content_callback_ = content_cb{this};
        f.content.changed.subscribe(content_slot_, content_callback_);
    }

private:
    detail::property_binding<color>     binding_border_color_{};
    detail::property_binding<bool>      binding_has_shadow_{};
    detail::property_binding<float>     binding_corner_radius_{};
    detail::property_binding<thickness> binding_padding_{};

    struct content_cb {
        frame_handler* self;
        void operator()(const std::shared_ptr<view>& v) const {
            self->record("content.present", v != nullptr);
        }
    };
    signal_slot<const std::shared_ptr<view>&> content_slot_{};
    content_cb                                content_callback_{this};
};

} // namespace mpapp::internal
#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif

#endif // MPAPP_HANDLERS_MOCK_FRAME_HANDLER_HPP
