// SPDX-License-Identifier: Apache-2.0
// GTK4 table_view handler implementation.

#include "mpapp/handlers/linux/table_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp {

table_view_handler<platform::linux_>::table_view_handler() {
    native_   = gtk_scrolled_window_new();
    list_box_ = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(static_cast<GtkWidget*>(list_box_)),
                                    GTK_SELECTION_SINGLE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(static_cast<GtkWidget*>(native_)),
                                  static_cast<GtkWidget*>(list_box_));
    gtk_widget_set_vexpand(static_cast<GtkWidget*>(native_), TRUE);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(native_), TRUE);
}

table_view_handler<platform::linux_>::~table_view_handler() = default;

void table_view_handler<platform::linux_>::rebuild_items(const std::vector<table_section_data>& sections) {
    GtkListBox* box = GTK_LIST_BOX(static_cast<GtkWidget*>(list_box_));
    GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(box));
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(box, child);
        child = next;
    }
    for (const auto& sec : sections) {
        // Section header row (non-selectable, distinct styling via leading marker).
        std::string header = "▾ " + sec.title;
        GtkWidget* hdr = gtk_label_new(header.c_str());
        gtk_widget_set_halign(hdr, GTK_ALIGN_START);
        GtkWidget* hdr_row = gtk_list_box_row_new();
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(hdr_row), hdr);
        gtk_list_box_row_set_selectable(GTK_LIST_BOX_ROW(hdr_row), FALSE);
        gtk_list_box_row_set_activatable(GTK_LIST_BOX_ROW(hdr_row), FALSE);
        gtk_list_box_append(box, hdr_row);

        for (const auto& row : sec.rows) {
            GtkWidget* lbl = gtk_label_new(row.c_str());
            gtk_widget_set_halign(lbl, GTK_ALIGN_START);
            gtk_list_box_append(box, lbl);
        }
    }
}

void table_view_handler<platform::linux_>::apply_row_height(int /*h*/) {
    // row_height honoring needs row-height CSS or per-row size_request;
    // not wired in v1.
}

void table_view_handler<platform::linux_>::map_sections(table_view& tv) {
    rebuild_items(tv.sections.get());
    tv.sections.changed.subscribe(sec_slot_, sec_cb_);
}

void table_view_handler<platform::linux_>::map_row_height(table_view& tv) {
    apply_row_height(tv.row_height.get());
    tv.row_height.changed.subscribe(rh_slot_, rh_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_table_view(::mpapp::view* v) {
    if (auto* t = dynamic_cast<::mpapp::table_view*>(v); t && t->has_tv_handler()) {
        return GTK_WIDGET(t->tv_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_table_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
