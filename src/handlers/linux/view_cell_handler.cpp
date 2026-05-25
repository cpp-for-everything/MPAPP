// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_view_cell handler implementation.

#include "mpapp/handlers/linux/view_cell_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

view_cell_handler<platform::linux_>::view_cell_handler() {
    native_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_start (static_cast<GtkWidget*>(native_), 12);
    gtk_widget_set_margin_end   (static_cast<GtkWidget*>(native_), 12);
    gtk_widget_set_margin_top   (static_cast<GtkWidget*>(native_), 6);
    gtk_widget_set_margin_bottom(static_cast<GtkWidget*>(native_), 6);
}

view_cell_handler<platform::linux_>::~view_cell_handler() = default;

void view_cell_handler<platform::linux_>::apply_content(view* v) {
    GtkBox* host = GTK_BOX(static_cast<GtkWidget*>(native_));
    if (current_child_ != nullptr) {
        gtk_box_remove(host, GTK_WIDGET(current_child_));
        current_child_ = nullptr;
    }
    if (v != nullptr) {
        if (GtkWidget* w = detail::linux_dispatch::dispatch(v); w != nullptr) {
            gtk_box_append(host, w);
            current_child_ = w;
        }
    }
}

void view_cell_handler<platform::linux_>::map_content(basic_view_cell& c) {
    apply_content(c.content.get());
    c.content.changed.subscribe(content_slot_, content_cb_);
}

void view_cell_handler<platform::linux_>::map_gestures(basic_view_cell& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_view_cell(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::internal::basic_view_cell*>(v); c && c->has_vc_handler()) {
        return GTK_WIDGET(c->vc_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_view_cell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
