// SPDX-License-Identifier: Apache-2.0
// GTK4 graphics_view handler implementation.
//
// The handler renders `graphics_view.drawable` into a facade canvas
// (currently Cairo) sized to the area's current dimensions, then blits
// the resulting BGRA32 pixels into the `GtkDrawingArea`'s `cairo_t`
// each paint. See the header for why off-screen + blit (rather than
// passing GTK's `cairo_t*` straight through the facade).

#include "mpapp/handlers/linux/graphics_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <cairo.h>
#include <gtk/gtk.h>

#include "mpapp/detail/graphics/canvas.hpp"
#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp {

namespace {

// Static GTK draw callback. user_data is the handler's `this`. Runs
// every time the area invalidates or resizes; falls back to a no-op
// clear if there's no bound graphics_view or no drawable set.
void on_draw_func(GtkDrawingArea* /*area*/,
                  cairo_t*        cr,
                  int             width,
                  int             height,
                  gpointer        user_data) {
    auto* h = static_cast<graphics_view_handler<platform::linux_>*>(user_data);
    if (h == nullptr) return;
    graphics_view* gv = h->bound();
    if (gv == nullptr || width <= 0 || height <= 0) return;
    const auto& cb = gv->drawable.get();
    if (!cb) {
        // No user callback installed — leave the area blank. (GTK
        // already clears to the theme background before invoking the
        // draw callback, so there's nothing to do.)
        return;
    }

    // Render into a facade canvas of (width, height). The facade owns
    // its surface; we read pixels back via the abstract API.
    auto canvas = detail::graphics::make_canvas(width, height);
    if (canvas == nullptr) return;
    cb(*canvas);

    // Blit the facade's pixels through GTK's cairo context. Cairo's
    // image-surface-for-data wraps an existing memory buffer with the
    // given format/stride — no copy. The wrapper surface is destroyed
    // at the end of the call; the underlying memory stays owned by
    // the facade canvas.
    const std::uint8_t* px     = canvas->pixel_data();
    const int           stride = canvas->pixel_stride_bytes();
    if (px == nullptr || stride <= 0) {
        // Backend has no readback (stub backend). Skip the blit.
        return;
    }
    cairo_surface_t* src = cairo_image_surface_create_for_data(
        const_cast<unsigned char*>(px),
        CAIRO_FORMAT_ARGB32,
        width,
        height,
        stride);
    if (src == nullptr) return;
    cairo_set_source_surface(cr, src, 0, 0);
    cairo_paint(cr);
    cairo_surface_destroy(src);
}

} // namespace

graphics_view_handler<platform::linux_>::graphics_view_handler() {
    native_ = gtk_drawing_area_new();
    // Install the draw function once. `this` is stable for the
    // handler's lifetime; the callback dereferences via bound() to
    // pick up the current graphics_view + drawable.
    gtk_drawing_area_set_draw_func(
        GTK_DRAWING_AREA(static_cast<GtkWidget*>(native_)),
        &on_draw_func,
        this,
        nullptr);
}

graphics_view_handler<platform::linux_>::~graphics_view_handler() = default;

void graphics_view_handler<platform::linux_>::apply_width(int w) {
    if (native_ == nullptr) return;
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(static_cast<GtkWidget*>(native_)), w);
}

void graphics_view_handler<platform::linux_>::apply_height(int h) {
    if (native_ == nullptr) return;
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(static_cast<GtkWidget*>(native_)), h);
}

void graphics_view_handler<platform::linux_>::queue_redraw() {
    if (native_ == nullptr) return;
    gtk_widget_queue_draw(GTK_WIDGET(static_cast<GtkWidget*>(native_)));
}

void graphics_view_handler<platform::linux_>::map_size(graphics_view& gv) {
    bound_ = &gv;
    apply_width(gv.width.get());
    apply_height(gv.height.get());
    gv.width.changed.subscribe(w_slot_, w_cb_);
    gv.height.changed.subscribe(h_slot_, h_cb_);
}

void graphics_view_handler<platform::linux_>::map_draw_count(graphics_view& gv) {
    // `draw_count` bumps each time the user calls `gv.invalidate()` —
    // mirror that into a GTK queue_draw so the area repaints.
    bound_ = &gv;
    gv.draw_count.changed.subscribe(count_slot_, count_cb_);
}

void graphics_view_handler<platform::linux_>::map_drawable(graphics_view& gv) {
    // Replacing the drawable callback should also force a repaint so
    // the new draw logic shows up immediately (rather than waiting for
    // the next external invalidate).
    bound_ = &gv;
    gv.drawable.changed.subscribe(drawable_slot_, drawable_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_graphics_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::graphics_view*>(v); w && w->has_gv_handler()) {
        return GTK_WIDGET(w->gv_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_graphics_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
