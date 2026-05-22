// SPDX-License-Identifier: Apache-2.0
// GTK4 graphics_view handler implementation.

#include "mpapp/handlers/linux/graphics_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp {

graphics_view_handler<platform::linux_>::graphics_view_handler() {
    native_ = gtk_drawing_area_new();
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

void graphics_view_handler<platform::linux_>::map_size(graphics_view& gv) {
    apply_width(gv.width.get());
    apply_height(gv.height.get());
    gv.width.changed.subscribe(w_slot_, w_cb_);
    gv.height.changed.subscribe(h_slot_, h_cb_);
}

void graphics_view_handler<platform::linux_>::map_draw_count(graphics_view& /*gv*/) {
    // No-op for v1; see Windows handler note.
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
