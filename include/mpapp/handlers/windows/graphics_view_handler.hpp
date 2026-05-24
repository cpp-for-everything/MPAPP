// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_graphics_view handler — wraps a muxc::Image whose Source is
// a WriteableBitmap that we repaint from the user's `drawable`
// callback via the ADR-0015 canvas facade. Each paint cycle:
//
//   1. Build a facade canvas of (width, height) — the active backend
//      (Cairo or Skia, both of which use BGRA32-premultiplied storage)
//      renders into its own buffer.
//   2. Memcpy that buffer into the WriteableBitmap's pixel buffer.
//      WinUI WriteableBitmap is BGRA8-premultiplied, matching the
//      facade's pixel_data() format exactly — no byte-order swap.
//   3. Invalidate the bitmap so the Image control picks up the new
//      pixels.
//
// The Image type (rather than muxc::Canvas in earlier scaffolding)
// is the supported WinUI primitive for "show a pixel buffer that I
// repaint imperatively". The dispatch registry returns the Image as
// a UIElement — same erased type as any other handler — so callers
// that walk the visual tree don't notice.

#ifndef MPAPP_HANDLERS_WINDOWS_GRAPHICS_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_GRAPHICS_VIEW_HANDLER_HPP

#include "../../internal/basic_graphics_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <cstddef>

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>

namespace mpapp::internal {

template <>
class graphics_view_handler<platform::windows> {
public:
    graphics_view_handler();
    ~graphics_view_handler();

    graphics_view_handler(const graphics_view_handler&)            = delete;
    graphics_view_handler& operator=(const graphics_view_handler&) = delete;
    graphics_view_handler(graphics_view_handler&&)                 = delete;
    graphics_view_handler& operator=(graphics_view_handler&&)      = delete;

    void map_size(basic_graphics_view& gv);
    void map_draw_count(basic_graphics_view& gv);
    void map_drawable(basic_graphics_view& gv);

    winrt::Microsoft::UI::Xaml::Controls::Image&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Image& native() const noexcept { return native_; }

private:
    void apply_width(int w);
    void apply_height(int h);
    void ensure_bitmap(int w, int h);
    void repaint();

    struct w_cb_t {
        graphics_view_handler<platform::windows>* self;
        void operator()(int v) const { self->apply_width(v); }
    };
    struct h_cb_t {
        graphics_view_handler<platform::windows>* self;
        void operator()(int v) const { self->apply_height(v); }
    };
    struct count_cb_t {
        graphics_view_handler<platform::windows>* self;
        void operator()(std::size_t /*v*/) const { self->repaint(); }
    };
    struct drawable_cb_t {
        graphics_view_handler<platform::windows>* self;
        void operator()(const basic_graphics_view::draw_callback_t& /*f*/) const { self->repaint(); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Image                native_{nullptr};
    winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap bitmap_{nullptr};
    int             bitmap_w_ = 0;
    int             bitmap_h_ = 0;
    basic_graphics_view*  bound_    = nullptr;

    w_cb_t        w_cb_{this};
    h_cb_t        h_cb_{this};
    count_cb_t    count_cb_{this};
    drawable_cb_t drawable_cb_{this};
    signal_slot<const int&>                            w_slot_{};
    signal_slot<const int&>                            h_slot_{};
    signal_slot<const std::size_t&>                    count_slot_{};
    signal_slot<const basic_graphics_view::draw_callback_t&> drawable_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_GRAPHICS_VIEW_HANDLER_HPP
