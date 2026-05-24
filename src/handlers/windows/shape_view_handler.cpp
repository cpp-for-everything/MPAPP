// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_shape_view handler implementation. See header for the
// design — Image + WriteableBitmap fed by the shared
// detail::graphics::render_shape_view helper (T-0031 phase 2).

#include "mpapp/handlers/windows/shape_view_handler.hpp"

#if defined(_WIN32)

#include <cstdint>
#include <cstring>

#include <robuffer.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>

#include "mpapp/detail/graphics/canvas.hpp"
#include "mpapp/detail/graphics/shape_renderer.hpp"
#include "mpapp/handlers/windows/widget_dispatch.hpp"

namespace mpapp::internal {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;
namespace muxm = ::winrt::Microsoft::UI::Xaml::Media;
namespace mxmi = ::winrt::Microsoft::UI::Xaml::Media::Imaging;

namespace {

// Pull the raw pixel pointer out of a WriteableBitmap's PixelBuffer.
// WriteableBitmap stores BGRA8 premultiplied — matches the abstract
// canvas pixel_data() format exactly, so the blit is a single
// memcpy per row with no channel reordering.
uint8_t* get_pixel_buffer_data(mxmi::WriteableBitmap const& wb) {
    auto buf = wb.PixelBuffer();
    auto bba = buf.as<::Windows::Storage::Streams::IBufferByteAccess>();
    uint8_t* px = nullptr;
    winrt::check_hresult(bba->Buffer(&px));
    return px;
}

} // namespace

shape_view_handler<platform::windows>::shape_view_handler() {
    native_ = muxc::Image{};
    // Stretch=None — the bitmap is sized to match the layout slot via
    // SizeChanged (see repaint), so 1:1 pixel display is what we want.
    // (Uniform / Fill would scale the rasterized output and blur it.)
    native_.Stretch(muxm::Stretch::None);
    // Give the Image a sensible default size so a basic_shape_view in a
    // layout that doesn't override sizing gets a non-zero allocation.
    // The same 200x80 default the legacy XAML-Shape handler used.
    native_.Width(200.0);
    native_.Height(80.0);
}

shape_view_handler<platform::windows>::~shape_view_handler() {
    if (native_ != nullptr && size_changed_token_.value != 0) {
        try { native_.SizeChanged(size_changed_token_); } catch (...) {}
        size_changed_token_ = {};
    }
}

void shape_view_handler<platform::windows>::ensure_bitmap(int w, int h) {
    if (w <= 0 || h <= 0) return;
    if (bitmap_ != nullptr && bitmap_w_ == w && bitmap_h_ == h) return;
    bitmap_   = mxmi::WriteableBitmap{w, h};
    bitmap_w_ = w;
    bitmap_h_ = h;
    native_.Source(bitmap_);
}

void shape_view_handler<platform::windows>::repaint() {
    if (native_ == nullptr || bound_ == nullptr) return;
    // Render at the Image's current pixel size. ActualWidth/Height
    // is in DIPs — fine for our render-then-display path; high-DPI
    // upscaling is a follow-up (would multiply by
    // XamlRoot().RasterizationScale()).
    int w = static_cast<int>(native_.ActualWidth());
    int h = static_cast<int>(native_.ActualHeight());
    if (w <= 0 || h <= 0) {
        // Pre-layout — fall back to the Width/Height that the
        // constructor set so we have something to draw at startup.
        w = static_cast<int>(native_.Width());
        h = static_cast<int>(native_.Height());
    }
    if (w <= 0 || h <= 0) return;

    auto canvas = detail::graphics::make_canvas(w, h);
    if (canvas == nullptr) return;
    canvas->clear(detail::graphics::color::rgba(0.0f, 0.0f, 0.0f, 0.0f));
    detail::graphics::render_shape_view(*canvas, *bound_, w, h);

    ensure_bitmap(w, h);
    const uint8_t* src        = canvas->pixel_data();
    const int      src_stride = canvas->pixel_stride_bytes();
    if (src == nullptr || src_stride <= 0) return;
    uint8_t*  dst        = get_pixel_buffer_data(bitmap_);
    const int dst_stride = w * 4;
    if (src_stride == dst_stride) {
        std::memcpy(dst, src, static_cast<std::size_t>(dst_stride) * h);
    } else {
        for (int y = 0; y < h; ++y) {
            std::memcpy(dst + y * dst_stride,
                        src + y * src_stride,
                        static_cast<std::size_t>(dst_stride));
        }
    }
    bitmap_.Invalidate();

    // Honor the surface's opacity Observable at the WinUI level too —
    // the render_shape_view helper already multiplies opacity into the
    // fill/stroke alpha, but setting it on the Image as well preserves
    // the legacy behavior where users could fade the whole control via
    // basic_shape_view.opacity even when fill/stroke are opaque.
    native_.Opacity(bound_->opacity.get());
}

void shape_view_handler<platform::windows>::map_kind(basic_shape_view& s) {
    bound_ = &s;
    // Subscribe to SizeChanged once (when the surface first binds) so
    // the canvas reallocates to the layout-assigned dimensions.
    if (native_ != nullptr && size_changed_token_.value == 0) {
        auto* self = this;
        size_changed_token_ = native_.SizeChanged(
            [self](winrt::Windows::Foundation::IInspectable const&,
                   mux::SizeChangedEventArgs const&) {
                self->repaint();
            });
    }
    repaint();
    s.kind.changed.subscribe(kind_slot_, cb_);
}
void shape_view_handler<platform::windows>::map_data(basic_shape_view& s) {
    bound_ = &s;
    s.data.changed.subscribe(data_slot_, cb_);
}
void shape_view_handler<platform::windows>::map_fill(basic_shape_view& s) {
    bound_ = &s;
    s.fill.changed.subscribe(fill_slot_, cb_);
}
void shape_view_handler<platform::windows>::map_stroke(basic_shape_view& s) {
    bound_ = &s;
    s.stroke.changed.subscribe(stroke_slot_, cb_);
}
void shape_view_handler<platform::windows>::map_stroke_thickness(basic_shape_view& s) {
    bound_ = &s;
    s.stroke_thickness.changed.subscribe(stroke_thick_slot_, cb_);
}
void shape_view_handler<platform::windows>::map_opacity(basic_shape_view& s) {
    bound_ = &s;
    if (native_ != nullptr) native_.Opacity(s.opacity.get());
    s.opacity.changed.subscribe(opacity_slot_, cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_shape_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_shape_view*>(v); w && w->has_sv_handler()) {
        return w->sv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_shape_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
