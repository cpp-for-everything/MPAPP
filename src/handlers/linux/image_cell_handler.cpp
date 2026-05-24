// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_image_cell handler implementation.

#include "mpapp/handlers/linux/image_cell_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

image_cell_handler<platform::linux_>::image_cell_handler() {
    native_       = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    image_w_      = gtk_image_new();
    text_box_     = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    text_label_   = gtk_label_new("");
    detail_label_ = gtk_label_new("");

    // Icon-size leading basic_image (40px square, matches cell-row aesthetic).
    gtk_image_set_pixel_size(GTK_IMAGE(static_cast<GtkWidget*>(image_w_)), 40);
    gtk_widget_set_valign(static_cast<GtkWidget*>(image_w_), GTK_ALIGN_CENTER);

    gtk_widget_set_halign(static_cast<GtkWidget*>(text_label_),   GTK_ALIGN_START);
    gtk_widget_set_halign(static_cast<GtkWidget*>(detail_label_), GTK_ALIGN_START);
    gtk_widget_set_visible(static_cast<GtkWidget*>(detail_label_), FALSE);
    gtk_widget_set_valign(static_cast<GtkWidget*>(text_box_), GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(text_box_), TRUE);

    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(text_box_)),
                   static_cast<GtkWidget*>(text_label_));
    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(text_box_)),
                   static_cast<GtkWidget*>(detail_label_));

    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(native_)),
                   static_cast<GtkWidget*>(image_w_));
    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(native_)),
                   static_cast<GtkWidget*>(text_box_));

    // Native row padding.
    gtk_widget_set_margin_start (static_cast<GtkWidget*>(native_), 12);
    gtk_widget_set_margin_end   (static_cast<GtkWidget*>(native_), 12);
    gtk_widget_set_margin_top   (static_cast<GtkWidget*>(native_), 6);
    gtk_widget_set_margin_bottom(static_cast<GtkWidget*>(native_), 6);
}

image_cell_handler<platform::linux_>::~image_cell_handler() = default;

void image_cell_handler<platform::linux_>::apply_text(const std::string& v) {
    if (text_label_ == nullptr) return;
    gtk_label_set_text(GTK_LABEL(static_cast<GtkWidget*>(text_label_)), v.c_str());
}

void image_cell_handler<platform::linux_>::apply_detail(const std::string& v) {
    if (detail_label_ == nullptr) return;
    gtk_label_set_text(GTK_LABEL(static_cast<GtkWidget*>(detail_label_)), v.c_str());
    gtk_widget_set_visible(static_cast<GtkWidget*>(detail_label_), v.empty() ? FALSE : TRUE);
}

void image_cell_handler<platform::linux_>::apply_image_uri(const std::string& v) {
    if (image_w_ == nullptr) return;
    GtkImage* img = GTK_IMAGE(static_cast<GtkWidget*>(image_w_));
    if (v.empty()) {
        gtk_image_clear(img);
        return;
    }
    // Treat "icon:foo" as a themed icon name (e.g. "icon:document-open");
    // anything else is a filesystem path.
    if (v.starts_with("icon:")) {
        gtk_image_set_from_icon_name(img, v.c_str() + 5);
    } else {
        gtk_image_set_from_file(img, v.c_str());
    }
}

void image_cell_handler<platform::linux_>::map_text(basic_image_cell& c) {
    apply_text(c.text.get());
    c.text.changed.subscribe(text_slot_, text_cb_);
}

void image_cell_handler<platform::linux_>::map_detail(basic_image_cell& c) {
    apply_detail(c.detail.get());
    c.detail.changed.subscribe(detail_slot_, detail_cb_);
}

void image_cell_handler<platform::linux_>::map_image_uri(basic_image_cell& c) {
    apply_image_uri(c.image_uri.get());
    c.image_uri.changed.subscribe(uri_slot_, uri_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_image_cell(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::internal::basic_image_cell*>(v); c && c->has_ic_handler()) {
        return GTK_WIDGET(c->ic_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_image_cell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
