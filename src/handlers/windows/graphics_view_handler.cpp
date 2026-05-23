// SPDX-License-Identifier: Apache-2.0
// WinUI 3 graphics_view handler implementation.
//
// The handler renders graphics_view::drawable through the ADR-0015
// canvas facade and blits the resulting pixel buffer into a
// muxc::Image's Source via a WriteableBitmap. WriteableBitmap stores
// pixels as BGRA8 premultiplied — matching the abstract canvas
// pixel_data() format exactly, so the blit is a single memcpy with no
// channel reordering.

#include "mpapp/handlers/windows/graphics_view_handler.hpp"

#if defined(_WIN32)

#include <cstring>

#include <robuffer.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>

#include "mpapp/detail/graphics/canvas.hpp"
#include "mpapp/handlers/windows/widget_dispatch.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;
namespace mxmi = ::winrt::Microsoft::UI::Xaml::Media::Imaging;

namespace {

// Pulls the raw pixel-data pointer out of a WriteableBitmap's
// PixelBuffer. WinRT exposes the buffer as an IBuffer; IBufferByteAccess
// is the COM-side interface that hands us the underlying byte pointer.
// The pointer aliases the WriteableBitmap's owned memory and stays
// valid for the lifetime of the bitmap (we never resize in place —
// any size change builds a fresh WriteableBitmap, see ensure_bitmap).
uint8_t* get_pixel_buffer_data(mxmi::WriteableBitmap const& wb) {
    auto buf = wb.PixelBuffer();
    auto bba = buf.as<::Windows::Storage::Streams::IBufferByteAccess>();
    uint8_t* px = nullptr;
    winrt::check_hresult(bba->Buffer(&px));
    return px;
}

} // namespace

graphics_view_handler<platform::windows>::graphics_view_handler() {
    native_ = muxc::Image{};
    // Stretch=None so the bitmap renders at its native pixel size.
    // Apps that want stretching can wrap the graphics_view in a layout
    // that resizes — at the canvas-facade level we honor the user's
    // width/height exactly.
    native_.Stretch(::winrt::Microsoft::UI::Xaml::Media::Stretch::None);
}

graphics_view_handler<platform::windows>::~graphics_view_handler() = default;

void graphics_view_handler<platform::windows>::apply_width(int w) {
    if (native_ == nullptr) return;
    native_.Width(static_cast<double>(w));
    // Any size change invalidates the current bitmap — repaint() will
    // ensure_bitmap to the new dimensions on next paint.
    repaint();
}

void graphics_view_handler<platform::windows>::apply_height(int h) {
    if (native_ == nullptr) return;
    native_.Height(static_cast<double>(h));
    repaint();
}

void graphics_view_handler<platform::windows>::ensure_bitmap(int w, int h) {
    if (w <= 0 || h <= 0) return;
    if (bitmap_ != nullptr && bitmap_w_ == w && bitmap_h_ == h) return;
    bitmap_   = mxmi::WriteableBitmap{w, h};
    bitmap_w_ = w;
    bitmap_h_ = h;
    native_.Source(bitmap_);
}

void graphics_view_handler<platform::windows>::repaint() {
    if (native_ == nullptr || bound_ == nullptr) return;
    const int w = bound_->width.get();
    const int h = bound_->height.get();
    if (w <= 0 || h <= 0) return;

    const auto& cb = bound_->drawable.get();
    if (!cb) {
        // No callback — clear to transparent and stop. (We still want
        // to make sure the Image control has *something* to display
        // so it doesn't keep showing stale pixels from a previous
        // drawable assignment.)
        ensure_bitmap(w, h);
        uint8_t* px = get_pixel_buffer_data(bitmap_);
        std::memset(px, 0, static_cast<std::size_t>(w) * h * 4);
        bitmap_.Invalidate();
        return;
    }

    // Render into a facade canvas. Backend (Cairo or Skia) writes
    // BGRA32 premultiplied; that matches WriteableBitmap's BGRA8
    // exactly, so we memcpy row-by-row (the facade may have a stride
    // larger than width*4 for alignment).
    auto canvas = detail::graphics::make_canvas(w, h);
    if (canvas == nullptr) return;
    cb(*canvas);

    ensure_bitmap(w, h);
    const uint8_t* src        = canvas->pixel_data();
    const int      src_stride = canvas->pixel_stride_bytes();
    if (src == nullptr || src_stride <= 0) {
        // Backend has no readable surface (stub). Leave the bitmap
        // whatever it was — bail without invalidating.
        return;
    }
    uint8_t* dst        = get_pixel_buffer_data(bitmap_);
    const int dst_stride = w * 4;  // WriteableBitmap is tightly packed
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
}

void graphics_view_handler<platform::windows>::map_size(graphics_view& gv) {
    bound_ = &gv;
    apply_width(gv.width.get());
    apply_height(gv.height.get());
    gv.width.changed.subscribe(w_slot_, w_cb_);
    gv.height.changed.subscribe(h_slot_, h_cb_);
}

void graphics_view_handler<platform::windows>::map_draw_count(graphics_view& gv) {
    // draw_count bumps each time the user calls gv.invalidate() — that's
    // our cue to repaint.
    bound_ = &gv;
    gv.draw_count.changed.subscribe(count_slot_, count_cb_);
}

void graphics_view_handler<platform::windows>::map_drawable(graphics_view& gv) {
    bound_ = &gv;
    gv.drawable.changed.subscribe(drawable_slot_, drawable_cb_);
    // Initial paint with the currently-installed callback so the
    // Image control isn't blank if the user assigns drawable before
    // the first invalidate().
    repaint();
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_graphics_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::graphics_view*>(v); w && w->has_gv_handler()) {
        return w->gv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_graphics_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
