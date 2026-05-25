// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_border handler — wraps `android.widget.FrameLayout`
// with a `GradientDrawable` background carrying the stroke + corner radii.

#ifndef MPAPP_HANDLERS_ANDROID_BORDER_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_BORDER_HANDLER_HPP

#include <memory>

#include "../../internal/basic_border.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class border_handler<platform::android> {
public:
    border_handler();
    ~border_handler();

    border_handler(const border_handler&)            = delete;
    border_handler& operator=(const border_handler&) = delete;
    border_handler(border_handler&&)                 = delete;
    border_handler& operator=(border_handler&&)      = delete;

    void map_content(basic_border& b);
    void map_padding(basic_border& b);
    void map_stroke(basic_border& b);
    void map_stroke_thickness(basic_border& b);
    void map_stroke_shape(basic_border& b);

    void bind_content(basic_border& b, view& child);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_border& /*x*/) noexcept {}


private:
    void apply_content(const std::shared_ptr<view>& v);
    void apply_padding(const thickness& t);
    void apply_stroke(const brush_ref& b);
    void apply_stroke_thickness(double t);
    void apply_stroke_shape(const stroke_shape_desc& s);
    void rebuild_background();

    struct content_cb_t       { border_handler<platform::android>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };
    struct padding_cb_t       { border_handler<platform::android>* self; void operator()(const thickness& t) const { self->apply_padding(t); } };
    struct stroke_cb_t        { border_handler<platform::android>* self; void operator()(const brush_ref& b) const { self->apply_stroke(b); } };
    struct stroke_thick_cb_t  { border_handler<platform::android>* self; void operator()(double t) const { self->apply_stroke_thickness(t); } };
    struct stroke_shape_cb_t  { border_handler<platform::android>* self; void operator()(const stroke_shape_desc& s) const { self->apply_stroke_shape(s); } };

    jobject native_ = nullptr;  // FrameLayout global ref

    brush_ref         cached_stroke_{};
    double            cached_stroke_thickness_ = 1.0;
    stroke_shape_desc cached_stroke_shape_{};

    content_cb_t                              content_cb_{this};
    padding_cb_t                              padding_cb_{this};
    stroke_cb_t                               stroke_cb_{this};
    stroke_thick_cb_t                         stroke_thick_cb_{this};
    stroke_shape_cb_t                         stroke_shape_cb_{this};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
    signal_slot<const thickness&>             padding_slot_{};
    signal_slot<const brush_ref&>             stroke_slot_{};
    signal_slot<const double&>                stroke_thick_slot_{};
    signal_slot<const stroke_shape_desc&>     stroke_shape_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_BORDER_HANDLER_HPP
