// SPDX-License-Identifier: Apache-2.0
// Android basic_shape_view handler. T-0031 phase 2: rendering goes through
// the shared detail::graphics::render_shape_view helper into an
// off-screen canvas; pixels are then byte-swapped (BGRA → RGBA byte
// order, matching ANDROID_BITMAP_FORMAT_RGBA_8888 in memory) and
// copied into an android.graphics.Bitmap that backs an
// android.widget.ImageView. The previous MppShapeView custom-Java-View
// path is gone — all three platforms now render through the same
// shared helper.
//
// The ImageView subscribes to View.OnLayoutChangeListener so the
// canvas reallocates + repaints whenever the layout assigns the
// basic_shape_view a new size, matching the auto-stretch behavior the
// previous custom View provided through its onMeasure / onDraw.

#ifndef MPAPP_HANDLERS_ANDROID_SHAPE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_SHAPE_VIEW_HANDLER_HPP

#include <cstddef>
#include <cstdint>
#include <string>

#include "../../platform.hpp"
#include "../../internal/basic_shape_view.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class shape_view_handler<platform::android> {
public:
    shape_view_handler();
    ~shape_view_handler();

    shape_view_handler(const shape_view_handler&)            = delete;
    shape_view_handler& operator=(const shape_view_handler&) = delete;
    shape_view_handler(shape_view_handler&&)                 = delete;
    shape_view_handler& operator=(shape_view_handler&&)      = delete;

    void map_kind(basic_shape_view& s);
    void map_data(basic_shape_view& s);
    void map_fill(basic_shape_view& s);
    void map_stroke(basic_shape_view& s);
    void map_stroke_thickness(basic_shape_view& s);
    void map_opacity(basic_shape_view& s);

    jobject native() const noexcept { return native_; }

    // Called from the OnLayoutChangeListener trampoline (in the .cpp).
    // Public so the trampoline can reach it without becoming a friend.
    void on_layout_changed(int w, int h);

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_shape_view& /*x*/) noexcept {}


private:
    void ensure_bitmap(int w, int h);
    void repaint();

    struct invalidate_cb_t {
        shape_view_handler<platform::android>* self;
        template <class T> void operator()(T const& /*v*/) const { self->repaint(); }
    };

    jobject     native_   = nullptr;   // android.widget.ImageView (global ref)
    jobject     bitmap_   = nullptr;   // android.graphics.Bitmap (global ref)
    int         bitmap_w_ = 0;
    int         bitmap_h_ = 0;
    int         layout_w_ = 0;
    int         layout_h_ = 0;
    basic_shape_view* bound_    = nullptr;

    invalidate_cb_t                 cb_{this};
    signal_slot<const shape_kind&>  kind_slot_{};
    signal_slot<const std::string&> data_slot_{};
    signal_slot<const std::string&> fill_slot_{};
    signal_slot<const std::string&> stroke_slot_{};
    signal_slot<const double&>      stroke_thick_slot_{};
    signal_slot<const double&>      opacity_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_SHAPE_VIEW_HANDLER_HPP
