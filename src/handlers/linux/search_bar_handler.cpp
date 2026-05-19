// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 search_bar handler implementation.

#include "mpapp/handlers/linux/search_bar_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp {

search_bar_handler<platform::linux_>::search_bar_handler() {
    native_ = gtk_search_entry_new();
}

search_bar_handler<platform::linux_>::~search_bar_handler() = default;

void search_bar_handler<platform::linux_>::apply_text(const std::string& v) {
    if (native_ == nullptr || suppress_echo_) return;
    suppress_echo_ = true;
    gtk_editable_set_text(GTK_EDITABLE(static_cast<GtkWidget*>(native_)), v.c_str());
    suppress_echo_ = false;
}

void search_bar_handler<platform::linux_>::apply_placeholder(const std::string& v) {
    if (native_ == nullptr) return;
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(static_cast<GtkWidget*>(native_)), v.c_str());
}

void search_bar_handler<platform::linux_>::map_text(search_bar& s) {
    apply_text(s.text.get());
    s.text.changed.subscribe(text_slot_, text_cb_);
}
void search_bar_handler<platform::linux_>::map_placeholder(search_bar& s) {
    apply_placeholder(s.placeholder.get());
    s.placeholder.changed.subscribe(placeholder_slot_, placeholder_cb_);
}

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
