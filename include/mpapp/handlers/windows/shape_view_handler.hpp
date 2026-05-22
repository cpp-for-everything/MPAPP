// SPDX-License-Identifier: Apache-2.0
// WinUI 3 shape_view handler — wraps a muxc::Border whose Child is one
// of muxc::Shapes::Rectangle / Ellipse / Line, chosen by `kind`. Hex
// color strings (`#RRGGBB` / `#AARRGGBB`) become SolidColorBrushes for
// fill / stroke. For v1, kinds `polygon` and `path` render as the
// outer bounding rectangle (full SVG path parsing is gated on the
// future graphics-backend ADR).

#ifndef MPAPP_HANDLERS_WINDOWS_SHAPE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_SHAPE_VIEW_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../shape_view.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>

namespace mpapp {

template <>
class shape_view_handler<platform::windows> {
public:
    shape_view_handler();
    ~shape_view_handler();

    shape_view_handler(const shape_view_handler&)            = delete;
    shape_view_handler& operator=(const shape_view_handler&) = delete;
    shape_view_handler(shape_view_handler&&)                 = delete;
    shape_view_handler& operator=(shape_view_handler&&)      = delete;

    void map_kind(shape_view& s);
    void map_data(shape_view& s);
    void map_fill(shape_view& s);
    void map_stroke(shape_view& s);
    void map_stroke_thickness(shape_view& s);
    void map_opacity(shape_view& s);

    winrt::Microsoft::UI::Xaml::Controls::Border&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Border& native() const noexcept { return native_; }

private:
    void rebuild_shape(shape_kind k);
    void apply_paint();
    void apply_data(const std::string& v);

    struct kind_cb_t {
        shape_view_handler<platform::windows>* self;
        void operator()(shape_kind v) const { self->rebuild_shape(v); }
    };
    struct data_cb_t {
        shape_view_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_data(v); }
    };
    struct fill_cb_t {
        shape_view_handler<platform::windows>* self;
        void operator()(const std::string&) const { self->apply_paint(); }
    };
    struct stroke_cb_t {
        shape_view_handler<platform::windows>* self;
        void operator()(const std::string&) const { self->apply_paint(); }
    };
    struct stroke_thick_cb_t {
        shape_view_handler<platform::windows>* self;
        void operator()(double) const { self->apply_paint(); }
    };
    struct opacity_cb_t {
        shape_view_handler<platform::windows>* self;
        void operator()(double v) const;
    };

    winrt::Microsoft::UI::Xaml::Controls::Border native_{nullptr};
    winrt::Microsoft::UI::Xaml::Shapes::Shape    shape_{nullptr};
    shape_view*                                  bound_ = nullptr;

    kind_cb_t                       kind_cb_{this};
    data_cb_t                       data_cb_{this};
    fill_cb_t                       fill_cb_{this};
    stroke_cb_t                     stroke_cb_{this};
    stroke_thick_cb_t               stroke_thick_cb_{this};
    opacity_cb_t                    opacity_cb_{this};
    signal_slot<const shape_kind&>  kind_slot_{};
    signal_slot<const std::string&> data_slot_{};
    signal_slot<const std::string&> fill_slot_{};
    signal_slot<const std::string&> stroke_slot_{};
    signal_slot<const double&>      stroke_thick_slot_{};
    signal_slot<const double&>      opacity_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_SHAPE_VIEW_HANDLER_HPP
