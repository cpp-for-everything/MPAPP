// SPDX-License-Identifier: Apache-2.0
// WinUI 3 shape_view handler. T-0031 phase 2: rendering goes through
// the shared detail::graphics::render_shape_view helper into an
// off-screen canvas; the resulting BGRA32 pixels are memcpy'd into a
// WriteableBitmap that backs an muxc::Image. The previous XAML
// muxs::Shape primitive approach (Rectangle / Ellipse / Line with a
// bounding-rect fallback for polygon + path) is gone — all platforms
// now render through the same helper, so output is identical.
//
// The Image subscribes to SizeChanged so the canvas reallocates +
// repaints whenever the layout assigns the shape_view a new size —
// matching the auto-stretch behavior the previous XAML Shape host
// provided.

#ifndef MPAPP_HANDLERS_WINDOWS_SHAPE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_SHAPE_VIEW_HANDLER_HPP

#include <cstddef>
#include <string>

#include "../../platform.hpp"
#include "../../shape_view.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>

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

    winrt::Microsoft::UI::Xaml::Controls::Image&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Image& native() const noexcept { return native_; }

private:
    void ensure_bitmap(int w, int h);
    void repaint();

    struct invalidate_cb_t {
        shape_view_handler<platform::windows>* self;
        // The same callback handles every observable — each just
        // triggers a full repaint. Inexpensive: render targets a
        // small (typically <500px) canvas; one paint per change is
        // fine.
        template <class T> void operator()(T const& /*v*/) const { self->repaint(); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Image                 native_{nullptr};
    winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap bitmap_{nullptr};
    int             bitmap_w_ = 0;
    int             bitmap_h_ = 0;
    winrt::event_token size_changed_token_{};
    shape_view*     bound_ = nullptr;

    invalidate_cb_t                 cb_{this};
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
