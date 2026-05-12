// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Border.md
//
// `border_handler<platform::mock>` — records property mappers for the
// `border` decorator. `content` is recorded as a presence flag (same
// reason as scroll_view); `stroke_dash_array` is recorded by length.

#ifndef MPAPP_HANDLERS_MOCK_BORDER_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_BORDER_HANDLER_HPP

#include "../../border.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class border_handler<platform::mock>
    : public mock_handler_base {
public:
    border_handler() = default;

    void map_padding(border& b)              { bind("padding",              b.padding,              binding_padding_); }
    void map_stroke_shape(border& b)         { bind("stroke_shape",         b.stroke_shape,         binding_stroke_shape_); }
    void map_stroke(border& b)               { bind("stroke",               b.stroke,               binding_stroke_); }
    void map_stroke_thickness(border& b)     { bind("stroke_thickness",     b.stroke_thickness,     binding_stroke_thickness_); }
    void map_stroke_dash_offset(border& b)   { bind("stroke_dash_offset",   b.stroke_dash_offset,   binding_dash_offset_); }
    void map_stroke_line_cap(border& b)      { bind("stroke_line_cap",      b.stroke_line_cap,      binding_line_cap_); }
    void map_stroke_line_join(border& b)     { bind("stroke_line_join",     b.stroke_line_join,     binding_line_join_); }
    void map_stroke_miter_limit(border& b)   { bind("stroke_miter_limit",   b.stroke_miter_limit,   binding_miter_limit_); }

    // shared_ptr<view> + std::vector<double> get presence/length-only treatment.
    void map_content(border& b) {
        record("content.present", b.content.get() != nullptr);
        content_callback_ = content_cb{this};
        b.content.changed.subscribe(content_slot_, content_callback_);
    }

    void map_stroke_dash_array(border& b) {
        record("stroke_dash_array.size", b.stroke_dash_array.get().size());
        dash_array_callback_ = dash_array_cb{this};
        b.stroke_dash_array.changed.subscribe(dash_array_slot_, dash_array_callback_);
    }

private:
    detail::property_binding<thickness>         binding_padding_{};
    detail::property_binding<stroke_shape_desc> binding_stroke_shape_{};
    detail::property_binding<brush_ref>         binding_stroke_{};
    detail::property_binding<double>            binding_stroke_thickness_{};
    detail::property_binding<double>            binding_dash_offset_{};
    detail::property_binding<pen_line_cap>      binding_line_cap_{};
    detail::property_binding<pen_line_join>     binding_line_join_{};
    detail::property_binding<double>            binding_miter_limit_{};

    struct content_cb {
        border_handler* self;
        void operator()(const std::shared_ptr<view>& v) const {
            self->record("content.present", v != nullptr);
        }
    };
    signal_slot<const std::shared_ptr<view>&> content_slot_{};
    content_cb                                content_callback_{this};

    struct dash_array_cb {
        border_handler* self;
        void operator()(const std::vector<double>& v) const {
            self->record("stroke_dash_array.size", v.size());
        }
    };
    signal_slot<const std::vector<double>&> dash_array_slot_{};
    dash_array_cb                           dash_array_callback_{this};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_BORDER_HANDLER_HPP
