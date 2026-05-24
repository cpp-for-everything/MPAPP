// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_table_view handler implementation.

#include "mpapp/handlers/linux/table_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/cell.hpp"
#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

// Decode a flat GtkListBox row index (section-header row + data rows
// concatenated) back to (section, row). Returns true if `position`
// lands on a data row; false if it's a header (or out of range).
template <class SectionVec>
bool decode_position(const SectionVec& sections,
                     int position,
                     int& out_section,
                     int& out_row) {
    int idx = position;
    for (std::size_t s = 0; s < sections.size(); ++s) {
        if (idx == 0) return false;
        idx -= 1;
        const int rows_in = static_cast<int>(sections[s].rows.size());
        if (idx < rows_in) {
            out_section = static_cast<int>(s);
            out_row     = idx;
            return true;
        }
        idx -= rows_in;
    }
    return false;
}

void on_row_selected(GtkListBox* box, GtkListBoxRow* row, gpointer user_data) {
    if (row == nullptr) return;
    auto* tv = static_cast<basic_table_view*>(user_data);
    if (tv == nullptr) return;
    const int flat = gtk_list_box_row_get_index(row);
    int section = 0, data_row = 0;
    const auto& typed = tv->typed_sections.get();
    const bool ok = !typed.empty()
        ? decode_position(typed, flat, section, data_row)
        : decode_position(tv->sections.get(), flat, section, data_row);
    // Clear selection so taps don't stick — matches MAUI TableView.
    gtk_list_box_unselect_all(box);
    if (!ok) return;

    tv->row_tapped.emit(section, data_row);
    if (cell* c = tv->cell_at(section, data_row); c != nullptr) {
        c->tapped.emit();
    }
}

void clear_listbox(GtkListBox* box) {
    GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(box));
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(box, child);
        child = next;
    }
}

void append_section_header(GtkListBox* box, const std::string& title) {
    const std::string header = "\xe2\x96\xbe " + title;  // "▾ " + title
    GtkWidget* hdr = gtk_label_new(header.c_str());
    gtk_widget_set_halign(hdr, GTK_ALIGN_START);
    GtkWidget* hdr_row = gtk_list_box_row_new();
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(hdr_row), hdr);
    gtk_list_box_row_set_selectable(GTK_LIST_BOX_ROW(hdr_row), FALSE);
    gtk_list_box_row_set_activatable(GTK_LIST_BOX_ROW(hdr_row), FALSE);
    gtk_list_box_append(box, hdr_row);
}

} // namespace

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
    clear_listbox(box);
    for (const auto& sec : sections) {
        append_section_header(box, sec.title);
        for (const auto& row : sec.rows) {
            GtkWidget* lbl = gtk_label_new(row.c_str());
            gtk_widget_set_halign(lbl, GTK_ALIGN_START);
            gtk_list_box_append(box, lbl);
        }
    }
}

void table_view_handler<platform::linux_>::rebuild_typed(const std::vector<table_section_typed>& sections) {
    GtkListBox* box = GTK_LIST_BOX(static_cast<GtkWidget*>(list_box_));
    clear_listbox(box);
    for (const auto& sec : sections) {
        append_section_header(box, sec.title);
        for (cell* c : sec.rows) {
            if (c == nullptr) continue;
            GtkWidget* w = detail::linux_dispatch::dispatch(c);
            if (w != nullptr) {
                gtk_list_box_append(box, w);
            }
        }
    }
}

void table_view_handler<platform::linux_>::rebuild_active() {
    if (bound_ == nullptr) return;
    const auto& typed = bound_->typed_sections.get();
    if (!typed.empty()) {
        rebuild_typed(typed);
    } else {
        rebuild_items(bound_->sections.get());
    }
}

void table_view_handler<platform::linux_>::apply_row_height(int /*h*/) {
    // row_height honoring needs row-height CSS or per-row size_request;
    // not wired in v1.
}

void table_view_handler<platform::linux_>::map_sections(basic_table_view& tv) {
    bound_ = &tv;
    rebuild_active();
    tv.sections.changed.subscribe(sec_slot_, sec_cb_);
    // Wire row taps once (idempotent across multiple map_* calls — the
    // signal handler is stable for the handler's lifetime).
    if (!tap_wired_) {
        g_signal_connect(static_cast<GtkWidget*>(list_box_), "row-selected",
                         G_CALLBACK(on_row_selected), &tv);
        tap_wired_ = true;
    }
}

void table_view_handler<platform::linux_>::map_typed_sections(basic_table_view& tv) {
    bound_ = &tv;
    rebuild_active();
    tv.typed_sections.changed.subscribe(typed_slot_, typed_cb_);
    if (!tap_wired_) {
        g_signal_connect(static_cast<GtkWidget*>(list_box_), "row-selected",
                         G_CALLBACK(on_row_selected), &tv);
        tap_wired_ = true;
    }
}

void table_view_handler<platform::linux_>::map_row_height(basic_table_view& tv) {
    apply_row_height(tv.row_height.get());
    tv.row_height.changed.subscribe(rh_slot_, rh_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_table_view(::mpapp::view* v) {
    if (auto* t = dynamic_cast<::mpapp::internal::basic_table_view*>(v); t && t->has_tv_handler()) {
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
