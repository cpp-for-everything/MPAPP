// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_shape_view handler implementation.
//
// T-0031 migration: rendering goes through the shared
// detail::graphics::render_shape_view helper (which writes into an
// ADR-0015 facade canvas) instead of issuing per-platform cairo calls
// directly. The GTK draw callback wraps the facade's pixel buffer
// via cairo_image_surface_create_for_data and blits it through GTK's
// cairo_t* — same pattern basic_graphics_view uses (T-0029 phase 1). Once
// Windows + Android basic_shape_view handlers follow in T-0031 phase 2,
// all three platforms render byte-for-byte identically (subject to
// the underlying canvas backend; default is Cairo on Linux/Win/Android,
// opt-in Skia via -DMPAPP_GRAPHICS_BACKEND=skia).

#include "mpapp/handlers/linux/shape_view_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <cstdint>

#include <cairo.h>
#include <gtk/gtk.h>

#include "mpapp/detail/graphics/canvas.hpp"
#include "mpapp/detail/graphics/shape_renderer.hpp"
#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

void draw_callback(GtkDrawingArea* /*area*/,
                   cairo_t* cr,
                   int width,
                   int height,
                   gpointer user_data) {
    auto* h = static_cast<shape_view_handler<platform::linux_>*>(user_data);
    if (h == nullptr || h->bound() == nullptr || width <= 0 || height <= 0) return;
    auto canvas = detail::graphics::make_canvas(width, height);
    if (canvas == nullptr) return;
    detail::graphics::render_shape_view(*canvas, *h->bound(), width, height);

    const std::uint8_t* px     = canvas->pixel_data();
    const int           stride = canvas->pixel_stride_bytes();
    if (px == nullptr || stride <= 0) return;
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

shape_view_handler<platform::linux_>::shape_view_handler() {
    native_ = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(
        GTK_DRAWING_AREA(static_cast<GtkWidget*>(native_)),
        draw_callback,
        this,
        nullptr);
    // GtkDrawingArea has no intrinsic size; give it a sensible default so
    // a basic_shape_view inside a basic_stack_layout (or any container that respects
    // child measure) gets a non-zero allocation. Apps can override by
    // setting the view's width/height once the handlers honor those.
    gtk_drawing_area_set_content_width (GTK_DRAWING_AREA(static_cast<GtkWidget*>(native_)), 200);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(static_cast<GtkWidget*>(native_)), 80);
}

shape_view_handler<platform::linux_>::~shape_view_handler() = default;

void shape_view_handler<platform::linux_>::invalidate_() {
    if (native_ != nullptr) {
        gtk_widget_queue_draw(static_cast<GtkWidget*>(native_));
    }
}

void shape_view_handler<platform::linux_>::map_kind(basic_shape_view& s) {
    bound_ = &s;
    invalidate_();
    s.kind.changed.subscribe(kind_slot_, kind_cb_);
}
void shape_view_handler<platform::linux_>::map_data(basic_shape_view& s) {
    invalidate_();
    s.data.changed.subscribe(data_slot_, data_cb_);
}
void shape_view_handler<platform::linux_>::map_fill(basic_shape_view& s) {
    invalidate_();
    s.fill.changed.subscribe(fill_slot_, fill_cb_);
}
void shape_view_handler<platform::linux_>::map_stroke(basic_shape_view& s) {
    invalidate_();
    s.stroke.changed.subscribe(stroke_slot_, stroke_cb_);
}
void shape_view_handler<platform::linux_>::map_stroke_thickness(basic_shape_view& s) {
    invalidate_();
    s.stroke_thickness.changed.subscribe(stroke_thick_slot_, stroke_thick_cb_);
}
void shape_view_handler<platform::linux_>::map_opacity(basic_shape_view& s) {
    invalidate_();
    s.opacity.changed.subscribe(opacity_slot_, opacity_cb_);
}

void shape_view_handler<platform::linux_>::map_gestures(basic_shape_view& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_shape_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_shape_view*>(v); w && w->has_sv_handler()) {
        return GTK_WIDGET(w->sv_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_shape_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
