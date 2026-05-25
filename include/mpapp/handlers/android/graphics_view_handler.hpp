// SPDX-License-Identifier: Apache-2.0
// Android basic_graphics_view handler — wraps an android.widget.ImageView
// whose ImageBitmap is repainted from the user's `drawable` callback
// via the ADR-0015 canvas facade. Each paint cycle:
//
//   1. Build a facade canvas of (width, height). The active backend
//      (Cairo or Skia) writes BGRA32 premultiplied pixels.
//   2. Allocate (or reuse) an android.graphics.Bitmap of size
//      (width, height) with Config.ARGB_8888 — which on Android is
//      stored in NDK ANDROID_BITMAP_FORMAT_RGBA_8888 byte order
//      (R, G, B, A) in memory.
//   3. AndroidBitmap_lockPixels + per-pixel swap bytes 0 and 2
//      (B <-> R) while copying into the bitmap buffer.
//   4. AndroidBitmap_unlockPixels + ImageView.setImageBitmap.
//
// The byte swap is the cost of the format difference. For typical
// basic_graphics_view sizes (a few hundred px square) it's negligible
// relative to the actual draw cost.

#ifndef MPAPP_HANDLERS_ANDROID_GRAPHICS_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_GRAPHICS_VIEW_HANDLER_HPP

#include <cstddef>

#include "../../internal/basic_graphics_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class graphics_view_handler<platform::android> {
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

    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_graphics_view& /*x*/) noexcept {}


private:
    void apply_width(int w);
    void apply_height(int h);
    void ensure_bitmap(int w, int h);
    void repaint();

    struct w_cb_t {
        graphics_view_handler<platform::android>* self;
        void operator()(int v) const { self->apply_width(v); }
    };
    struct h_cb_t {
        graphics_view_handler<platform::android>* self;
        void operator()(int v) const { self->apply_height(v); }
    };
    struct count_cb_t {
        graphics_view_handler<platform::android>* self;
        void operator()(std::size_t /*v*/) const { self->repaint(); }
    };
    struct drawable_cb_t {
        graphics_view_handler<platform::android>* self;
        void operator()(const basic_graphics_view::draw_callback_t& /*f*/) const { self->repaint(); }
    };

    jobject        native_   = nullptr;   // ImageView
    jobject        bitmap_   = nullptr;   // android.graphics.Bitmap (global ref)
    int            bitmap_w_ = 0;
    int            bitmap_h_ = 0;
    basic_graphics_view* bound_    = nullptr;

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
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_GRAPHICS_VIEW_HANDLER_HPP
