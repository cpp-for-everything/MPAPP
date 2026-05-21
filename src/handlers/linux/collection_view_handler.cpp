// SPDX-License-Identifier: Apache-2.0
// GTK4 collection_view handler implementation.

#include "mpapp/handlers/linux/collection_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp {

namespace {

void on_row_selected(GtkListBox* box, GtkListBoxRow* row, gpointer user_data) {
    auto* cv = static_cast<collection_view*>(user_data);
    if (cv == nullptr) return;
    if (g_object_get_data(G_OBJECT(box), "mpapp_suppress") != nullptr) return;
    int idx = (row != nullptr) ? gtk_list_box_row_get_index(row) : -1;
    if (cv->selected_index.get() != idx) cv->selected_index.set(idx);
    if (idx >= 0) cv->item_tapped.emit(idx);
}

void set_suppress(void* list_box, bool on) {
    g_object_set_data(G_OBJECT(static_cast<GtkWidget*>(list_box)),
                      "mpapp_suppress",
                      on ? GINT_TO_POINTER(1) : nullptr);
}

} // namespace

collection_view_handler<platform::linux_>::collection_view_handler() {
    native_   = gtk_scrolled_window_new();
    list_box_ = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(static_cast<GtkWidget*>(list_box_)),
                                    GTK_SELECTION_SINGLE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(static_cast<GtkWidget*>(native_)),
                                  static_cast<GtkWidget*>(list_box_));
    gtk_widget_set_vexpand(static_cast<GtkWidget*>(native_), TRUE);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(native_), TRUE);
}

collection_view_handler<platform::linux_>::~collection_view_handler() = default;

void collection_view_handler<platform::linux_>::rebuild_items(const std::vector<std::string>& v) {
    GtkListBox* box = GTK_LIST_BOX(static_cast<GtkWidget*>(list_box_));
    set_suppress(list_box_, true);
    GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(box));
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(box, child);
        child = next;
    }
    for (const auto& s : v) {
        GtkWidget* lbl = gtk_label_new(s.c_str());
        gtk_widget_set_halign(lbl, GTK_ALIGN_START);
        gtk_list_box_append(box, lbl);
    }
    if (bound_ != nullptr) apply_selection(bound_->selected_index.get());
    set_suppress(list_box_, false);
}

void collection_view_handler<platform::linux_>::apply_selection(int idx) {
    if (list_box_ == nullptr) return;
    GtkListBox* box = GTK_LIST_BOX(static_cast<GtkWidget*>(list_box_));
    set_suppress(list_box_, true);
    if (idx < 0) {
        gtk_list_box_unselect_all(box);
    } else {
        GtkListBoxRow* row = gtk_list_box_get_row_at_index(box, idx);
        if (row != nullptr) gtk_list_box_select_row(box, row);
    }
    set_suppress(list_box_, false);
}

void collection_view_handler<platform::linux_>::apply_selection_mode(collection_selection_mode m) {
    if (list_box_ == nullptr) return;
    GtkListBox* box = GTK_LIST_BOX(static_cast<GtkWidget*>(list_box_));
    switch (m) {
        case collection_selection_mode::none:
            gtk_list_box_set_selection_mode(box, GTK_SELECTION_NONE);
            break;
        case collection_selection_mode::multiple:
            gtk_list_box_set_selection_mode(box, GTK_SELECTION_MULTIPLE);
            break;
        case collection_selection_mode::single:
        default:
            gtk_list_box_set_selection_mode(box, GTK_SELECTION_SINGLE);
            break;
    }
}

void collection_view_handler<platform::linux_>::map_items_source(collection_view& cv) {
    bound_ = &cv;
    g_signal_connect(static_cast<GtkWidget*>(list_box_), "row-selected",
                     G_CALLBACK(on_row_selected), &cv);
    rebuild_items(cv.items_source.get());
    cv.items_source.changed.subscribe(items_slot_, items_cb_);
}

void collection_view_handler<platform::linux_>::map_selected_index(collection_view& cv) {
    apply_selection(cv.selected_index.get());
    cv.selected_index.changed.subscribe(sel_slot_, sel_cb_);
}

void collection_view_handler<platform::linux_>::map_selection_mode(collection_view& cv) {
    apply_selection_mode(cv.selection_mode.get());
    cv.selection_mode.changed.subscribe(mode_slot_, mode_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_collection_view(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::collection_view*>(v); c && c->has_cv_handler()) {
        return GTK_WIDGET(c->cv_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_collection_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
