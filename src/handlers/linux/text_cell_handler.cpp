// SPDX-License-Identifier: Apache-2.0
// GTK4 text_cell handler implementation.

#include "mpapp/handlers/linux/text_cell_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp {

text_cell_handler<platform::linux_>::text_cell_handler() {
    native_       = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    text_label_   = gtk_label_new("");
    detail_label_ = gtk_label_new("");

    gtk_widget_set_halign(static_cast<GtkWidget*>(text_label_),   GTK_ALIGN_START);
    gtk_widget_set_halign(static_cast<GtkWidget*>(detail_label_), GTK_ALIGN_START);

    // Detail starts hidden — apply_detail toggles visibility when non-empty.
    gtk_widget_set_visible(static_cast<GtkWidget*>(detail_label_), FALSE);

    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(native_)),
                   static_cast<GtkWidget*>(text_label_));
    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(native_)),
                   static_cast<GtkWidget*>(detail_label_));

    // Native row styling: 12px horizontal / 6px vertical padding via
    // widget margins.
    gtk_widget_set_margin_start (static_cast<GtkWidget*>(native_), 12);
    gtk_widget_set_margin_end   (static_cast<GtkWidget*>(native_), 12);
    gtk_widget_set_margin_top   (static_cast<GtkWidget*>(native_), 6);
    gtk_widget_set_margin_bottom(static_cast<GtkWidget*>(native_), 6);
}

text_cell_handler<platform::linux_>::~text_cell_handler() = default;

void text_cell_handler<platform::linux_>::apply_text(const std::string& v) {
    if (text_label_ == nullptr) return;
    gtk_label_set_text(GTK_LABEL(static_cast<GtkWidget*>(text_label_)), v.c_str());
}

void text_cell_handler<platform::linux_>::apply_detail(const std::string& v) {
    if (detail_label_ == nullptr) return;
    gtk_label_set_text(GTK_LABEL(static_cast<GtkWidget*>(detail_label_)), v.c_str());
    gtk_widget_set_visible(static_cast<GtkWidget*>(detail_label_), v.empty() ? FALSE : TRUE);
}

void text_cell_handler<platform::linux_>::map_text(text_cell& c) {
    apply_text(c.text.get());
    c.text.changed.subscribe(text_slot_, text_cb_);
}

void text_cell_handler<platform::linux_>::map_detail(text_cell& c) {
    apply_detail(c.detail.get());
    c.detail.changed.subscribe(detail_slot_, detail_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_text_cell(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::text_cell*>(v); c && c->has_tc_handler()) {
        return GTK_WIDGET(c->tc_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_text_cell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
