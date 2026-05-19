// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 border handler — wraps `mux::Controls::Border`,
// which exposes Background, BorderBrush, BorderThickness, CornerRadius,
// Padding, and Child natively.

#ifndef MPAPP_HANDLERS_WINDOWS_BORDER_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_BORDER_HANDLER_HPP

#include <memory>

#include "../../border.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.h>

namespace mpapp {

template <>
class border_handler<platform::windows> {
public:
    border_handler();
    ~border_handler();

    border_handler(const border_handler&)            = delete;
    border_handler& operator=(const border_handler&) = delete;
    border_handler(border_handler&&)                 = delete;
    border_handler& operator=(border_handler&&)      = delete;

    void map_content(border& b);
    void map_padding(border& b);
    void map_stroke(border& b);
    void map_stroke_thickness(border& b);
    void map_stroke_shape(border& b);

    void bind_content(border& b, view& child);

    winrt::Microsoft::UI::Xaml::Controls::Border&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Border& native() const noexcept { return native_; }

private:
    void apply_content(const std::shared_ptr<view>& v);
    void apply_padding(const thickness& t);
    void apply_stroke(const brush_ref& b);
    void apply_stroke_thickness(double t);
    void apply_stroke_shape(const stroke_shape_desc& s);

    struct content_cb_t       { border_handler<platform::windows>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };
    struct padding_cb_t       { border_handler<platform::windows>* self; void operator()(const thickness& t) const { self->apply_padding(t); } };
    struct stroke_cb_t        { border_handler<platform::windows>* self; void operator()(const brush_ref& b) const { self->apply_stroke(b); } };
    struct stroke_thick_cb_t  { border_handler<platform::windows>* self; void operator()(double t) const { self->apply_stroke_thickness(t); } };
    struct stroke_shape_cb_t  { border_handler<platform::windows>* self; void operator()(const stroke_shape_desc& s) const { self->apply_stroke_shape(s); } };

    winrt::Microsoft::UI::Xaml::Controls::Border native_{nullptr};

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

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_BORDER_HANDLER_HPP
