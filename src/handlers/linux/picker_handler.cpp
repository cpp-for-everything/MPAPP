// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 picker handler implementation.

#include "mpapp/handlers/linux/picker_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <vector>

#include <gtk/gtk.h>

namespace mpapp {

picker_handler<platform::linux_>::picker_handler() {
    // Empty string list initially; items() populates it.
    GtkStringList* list = gtk_string_list_new(nullptr);
    string_list_ = list;
    GtkWidget* drop = gtk_drop_down_new(G_LIST_MODEL(list), nullptr);
    native_ = drop;
}

picker_handler<platform::linux_>::~picker_handler() = default;

void picker_handler<platform::linux_>::apply_items(const std::vector<std::string>& v) {
    if (native_ == nullptr || string_list_ == nullptr || suppress_echo_) return;
    suppress_echo_ = true;
    // Rebuild via splice(0, n_items, new).
    GtkStringList* list = GTK_STRING_LIST(string_list_);
    const guint old_count = g_list_model_get_n_items(G_LIST_MODEL(list));
    std::vector<const char*> c_strs;
    c_strs.reserve(v.size() + 1);
    for (const auto& s : v) c_strs.push_back(s.c_str());
    c_strs.push_back(nullptr);
    gtk_string_list_splice(list, 0, old_count, c_strs.data());
    suppress_echo_ = false;
}

void picker_handler<platform::linux_>::apply_selected_index(int v) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    gtk_drop_down_set_selected(GTK_DROP_DOWN(static_cast<GtkWidget*>(native_)),
                               v < 0 ? GTK_INVALID_LIST_POSITION : static_cast<guint>(v));
    suppress_echo_ = false;
}

void picker_handler<platform::linux_>::map_items(picker& p) {
    apply_items(p.items.get());
    p.items.changed.subscribe(items_slot_, items_cb_);
}
void picker_handler<platform::linux_>::map_selected_index(picker& p) {
    apply_selected_index(p.selected_index.get());
    p.selected_index.changed.subscribe(selected_slot_, selected_cb_);
}
void picker_handler<platform::linux_>::map_title(picker& p) {
    apply_title(p.title.get());
    p.title.changed.subscribe(title_slot_, title_cb_);
}

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
