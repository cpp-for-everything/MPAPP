// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_graphics_view handler — wraps a GtkDrawingArea and pumps a
// user-supplied `drawable` callback through the ADR-0015 canvas facade
// each time GTK asks the area to paint. The handler renders into a
// facade-owned off-screen surface (currently Cairo when the Cairo
// backend is selected), then blits the resulting BGRA32 pixels into
// the GtkDrawingArea's `cairo_t` via `cairo_image_surface_create_for_data`.
//
// Why off-screen + blit rather than passing GTK's `cairo_t*` straight
// through the facade: the facade is backend-agnostic, so its API can't
// take a Cairo-specific handle as a parameter. Off-screen + blit
// preserves portability and matches what the Windows / Android handlers
// will do in their follow-up sessions (their native paint surfaces
// don't accept a Cairo context either).

#ifndef MPAPP_HANDLERS_LINUX_GRAPHICS_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_GRAPHICS_VIEW_HANDLER_HPP

#include <cstddef>

#include "../../internal/basic_graphics_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class graphics_view_handler<platform::linux_> {
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

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

    // Public so the static GTK draw callback (in the .cpp) can read
    // the bound basic_graphics_view. Marked here rather than as a friend so
    // the .cpp can stay free-function (matches existing dispatch
    // patterns in this codebase).
    basic_graphics_view* bound() const noexcept { return bound_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_graphics_view& x);


private:
    void apply_width(int w);
    void apply_height(int h);
    void queue_redraw();

    struct w_cb_t {
        graphics_view_handler<platform::linux_>* self;
        void operator()(int v) const { self->apply_width(v); }
    };
    struct h_cb_t {
        graphics_view_handler<platform::linux_>* self;
        void operator()(int v) const { self->apply_height(v); }
    };
    struct count_cb_t {
        graphics_view_handler<platform::linux_>* self;
        void operator()(std::size_t /*v*/) const { self->queue_redraw(); }
    };
    struct drawable_cb_t {
        graphics_view_handler<platform::linux_>* self;
        void operator()(const basic_graphics_view::draw_callback_t& /*f*/) const {
            self->queue_redraw();
        }
    };

    void*           native_ = nullptr;  // GtkDrawingArea*
    basic_graphics_view*  bound_  = nullptr;

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
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_GRAPHICS_VIEW_HANDLER_HPP
