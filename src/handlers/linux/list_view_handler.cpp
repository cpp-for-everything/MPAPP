// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_list_view handler implementation.

#include "mpapp/handlers/linux/list_view_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

// User-selection callback. The GtkListBox row-selected signal fires both
// for user clicks AND programmatic `gtk_list_box_select_row` calls. To
// avoid an infinite loop with the apply_selection path, we stash a
// transient "suppress" flag on the box object itself.
//
// `user_data` is the bound basic_list_view*. Selecting before the items_source
// map → bound is nullptr → we no-op.
void on_row_selected(GtkListBox* box, GtkListBoxRow* row, gpointer user_data) {
    auto* lv = static_cast<basic_list_view*>(user_data);
    if (lv == nullptr) return;
    if (g_object_get_data(G_OBJECT(box), "mpapp_suppress") != nullptr) return;
    int idx = (row != nullptr) ? gtk_list_box_row_get_index(row) : -1;
    if (lv->selected_index.get() != idx) {
        lv->selected_index.set(idx);
    }
    if (idx >= 0) lv->item_tapped.emit(idx);
}

void set_suppress(void* list_box, bool on) {
    g_object_set_data(G_OBJECT(static_cast<GtkWidget*>(list_box)),
                      "mpapp_suppress",
                      on ? GINT_TO_POINTER(1) : nullptr);
}

} // namespace

list_view_handler<platform::linux_>::list_view_handler() {
    native_   = gtk_scrolled_window_new();
    list_box_ = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(static_cast<GtkWidget*>(list_box_)),
                                    GTK_SELECTION_SINGLE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(static_cast<GtkWidget*>(native_)),
                                  static_cast<GtkWidget*>(list_box_));
    gtk_widget_set_vexpand(static_cast<GtkWidget*>(native_), TRUE);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(native_), TRUE);
    // The row-selected handler is connected in map_items_source once
    // we know which basic_list_view to forward to.
}

list_view_handler<platform::linux_>::~list_view_handler() = default;

void list_view_handler<platform::linux_>::rebuild_items(const std::vector<std::string>& v) {
    GtkListBox* box = GTK_LIST_BOX(static_cast<GtkWidget*>(list_box_));
    set_suppress(list_box_, true);
    // Remove all current rows.
    GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(box));
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(box, child);
        child = next;
    }
    // Append new rows.
    for (const auto& s : v) {
        GtkWidget* lbl = gtk_label_new(s.c_str());
        gtk_widget_set_halign(lbl, GTK_ALIGN_START);
        gtk_list_box_append(box, lbl);
    }
    if (bound_ != nullptr) {
        // Restore selection.
        int idx = bound_->selected_index.get();
        if (idx >= 0 && idx < static_cast<int>(v.size())) {
            GtkListBoxRow* row = gtk_list_box_get_row_at_index(box, idx);
            if (row != nullptr) gtk_list_box_select_row(box, row);
        }
    }
    set_suppress(list_box_, false);
}

void list_view_handler<platform::linux_>::apply_selection(int idx) {
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

void list_view_handler<platform::linux_>::map_items_source(basic_list_view& lv) {
    bound_ = &lv;
    // Connect the row-selected signal now that we know who to forward to.
    g_signal_connect(static_cast<GtkWidget*>(list_box_), "row-selected",
                     G_CALLBACK(on_row_selected), &lv);
    rebuild_items(lv.items_source.get());
    lv.items_source.changed.subscribe(items_slot_, items_cb_);
}

void list_view_handler<platform::linux_>::map_selected_index(basic_list_view& lv) {
    apply_selection(lv.selected_index.get());
    lv.selected_index.changed.subscribe(sel_slot_, sel_cb_);
}

void list_view_handler<platform::linux_>::map_gestures(basic_list_view& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_list_view(::mpapp::view* v) {
    if (auto* l = dynamic_cast<::mpapp::internal::basic_list_view*>(v); l && l->has_lv_handler()) {
        return GTK_WIDGET(l->lv_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_list_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
