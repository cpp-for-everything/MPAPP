// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_shell handler implementation.

#include "mpapp/handlers/linux/shell_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

void on_tab_clicked(GtkButton* /*btn*/, gpointer user_data) {
    auto* pair = static_cast<std::pair<basic_shell*, int>*>(user_data);
    if (pair == nullptr || pair->first == nullptr) return;
    pair->first->current_tab_index.set(pair->second);
}

} // namespace

shell_handler<platform::linux_>::shell_handler() {
    native_ = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(native_), TRUE);
    gtk_widget_set_vexpand(static_cast<GtkWidget*>(native_), TRUE);

    flyout_host_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    main_host_   = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    tab_strip_   = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    content_host_= gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    gtk_widget_set_vexpand(static_cast<GtkWidget*>(content_host_), TRUE);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(main_host_),    TRUE);

    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(main_host_)),
                   static_cast<GtkWidget*>(tab_strip_));
    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(main_host_)),
                   static_cast<GtkWidget*>(content_host_));

    gtk_paned_set_start_child(GTK_PANED(static_cast<GtkWidget*>(native_)),
                              static_cast<GtkWidget*>(flyout_host_));
    gtk_paned_set_end_child  (GTK_PANED(static_cast<GtkWidget*>(native_)),
                              static_cast<GtkWidget*>(main_host_));

    gtk_widget_set_visible(static_cast<GtkWidget*>(flyout_host_), FALSE);
}

shell_handler<platform::linux_>::~shell_handler() = default;

void shell_handler<platform::linux_>::rebuild_tab_strip(const std::vector<std::string>& labels) {
    GtkBox* strip = GTK_BOX(static_cast<GtkWidget*>(tab_strip_));
    // Remove all current children.
    GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(strip));
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(strip, child);
        child = next;
    }
    // Add fresh buttons.
    for (std::size_t i = 0; i < labels.size(); ++i) {
        GtkWidget* btn = gtk_button_new_with_label(labels[i].c_str());
        // Store (basic_shell*, idx) per basic_button; freed when basic_button is destroyed.
        auto* pair = new std::pair<basic_shell*, int>{bound_, static_cast<int>(i)};
        g_object_set_data_full(G_OBJECT(btn), "mpapp_tab_index", pair,
                               [](gpointer p){ delete static_cast<std::pair<basic_shell*, int>*>(p); });
        g_signal_connect(btn, "clicked", G_CALLBACK(on_tab_clicked), pair);
        gtk_box_append(strip, btn);
    }
}

void shell_handler<platform::linux_>::apply_selection(int /*idx*/) {
    // Visual highlight of the active tab is deferred to a styling pass.
}

void shell_handler<platform::linux_>::apply_is_flyout_open(bool v) {
    if (flyout_host_ == nullptr) return;
    gtk_widget_set_visible(static_cast<GtkWidget*>(flyout_host_), v ? TRUE : FALSE);
}

void shell_handler<platform::linux_>::apply_flyout_content(basic_page* p) {
    GtkBox* host = GTK_BOX(static_cast<GtkWidget*>(flyout_host_));
    if (current_flyout_child_ != nullptr) {
        gtk_box_remove(host, GTK_WIDGET(current_flyout_child_));
        current_flyout_child_ = nullptr;
    }
    if (p != nullptr) {
        if (GtkWidget* w = detail::linux_dispatch::dispatch(p); w != nullptr) {
            gtk_box_append(host, w);
            current_flyout_child_ = w;
        }
    }
}

void shell_handler<platform::linux_>::apply_current_content(basic_page* p) {
    GtkBox* host = GTK_BOX(static_cast<GtkWidget*>(content_host_));
    if (current_content_child_ != nullptr) {
        gtk_box_remove(host, GTK_WIDGET(current_content_child_));
        current_content_child_ = nullptr;
    }
    if (p != nullptr) {
        if (GtkWidget* w = detail::linux_dispatch::dispatch(p); w != nullptr) {
            gtk_box_append(host, w);
            current_content_child_ = w;
        }
    }
}

void shell_handler<platform::linux_>::map_tabs(basic_shell& s) {
    bound_ = &s;
    rebuild_tab_strip(s.tabs.get());
    s.tabs.changed.subscribe(tabs_slot_, tabs_cb_);
}

void shell_handler<platform::linux_>::map_current_tab_index(basic_shell& s) {
    apply_selection(s.current_tab_index.get());
    s.current_tab_index.changed.subscribe(sel_slot_, sel_cb_);
}

void shell_handler<platform::linux_>::map_is_flyout_open(basic_shell& s) {
    apply_is_flyout_open(s.is_flyout_open.get());
    s.is_flyout_open.changed.subscribe(flyout_open_slot_, flyout_open_cb_);
}

void shell_handler<platform::linux_>::map_flyout_content(basic_shell& s) {
    apply_flyout_content(s.flyout_content.get());
    s.flyout_content.changed.subscribe(flyout_content_slot_, flyout_content_cb_);
}

void shell_handler<platform::linux_>::map_current_content(basic_shell& s) {
    apply_current_content(s.current_content.get());
    s.current_content.changed.subscribe(content_slot_, content_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_shell(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::internal::basic_shell*>(v); s && s->has_shell_handler()) {
        return GTK_WIDGET(s->shell_handler_ref().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_shell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
