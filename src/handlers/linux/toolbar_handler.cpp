// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 toolbar handler implementation.

#include "mpapp/handlers/linux/toolbar_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <vector>

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp {

namespace {

// Remove every child packed into either side of a GtkActionBar. We snapshot
// the leading side via GtkWidget's child iteration because GtkActionBar
// itself doesn't expose a "clear" helper.
void action_bar_clear_pack_start(GtkActionBar* bar) {
    GtkWidget* w = gtk_widget_get_first_child(GTK_WIDGET(bar));
    std::vector<GtkWidget*> to_remove;
    while (w != nullptr) {
        // Only remove widgets we packed via pack_start; the center widget is
        // a sibling at the same level. We tag our buttons with a CSS class
        // so we can identify them safely.
        if (gtk_widget_has_css_class(w, "mpapp-toolbar-item")) {
            to_remove.push_back(w);
        }
        w = gtk_widget_get_next_sibling(w);
    }
    for (GtkWidget* victim : to_remove) {
        gtk_action_bar_remove(bar, victim);
    }
}

} // namespace

toolbar_handler<platform::linux_>::toolbar_handler() {
    GtkWidget* bar = gtk_action_bar_new();
    native_ = bar;
    // Empty center label by default — apply_title fills it in.
    GtkWidget* lbl = gtk_label_new("");
    title_label_ = lbl;
    gtk_action_bar_set_center_widget(GTK_ACTION_BAR(bar), lbl);
    gtk_action_bar_set_revealed(GTK_ACTION_BAR(bar), TRUE);
}

toolbar_handler<platform::linux_>::~toolbar_handler() = default;

void toolbar_handler<platform::linux_>::apply_items(const std::vector<toolbar_item>& v) {
    if (native_ == nullptr) return;
    GtkActionBar* bar = GTK_ACTION_BAR(static_cast<GtkWidget*>(native_));
    action_bar_clear_pack_start(bar);
    for (const auto& item : v) {
        GtkWidget* btn = gtk_button_new_with_label(item.text.c_str());
        gtk_widget_add_css_class(btn, "mpapp-toolbar-item");
        // `icon` is interpreted as a themed icon name when non-empty.
        if (!item.icon.empty()) {
            GtkWidget* image = gtk_image_new_from_icon_name(item.icon.c_str());
            gtk_button_set_child(GTK_BUTTON(btn), image);
        }
        gtk_action_bar_pack_start(bar, btn);
    }
}

void toolbar_handler<platform::linux_>::apply_title(const std::string& v) {
    if (title_label_ == nullptr) return;
    gtk_label_set_text(GTK_LABEL(static_cast<GtkWidget*>(title_label_)), v.c_str());
}

void toolbar_handler<platform::linux_>::map_items(toolbar& t) {
    apply_items(t.items.get());
    t.items.changed.subscribe(items_slot_, items_cb_);
}
void toolbar_handler<platform::linux_>::map_title(toolbar& t) {
    apply_title(t.title.get());
    t.title.changed.subscribe(title_slot_, title_cb_);
}

} // namespace mpapp

namespace {

// Per ADR-0013 — self-register so container dispatch sites resolve
// `mpapp::view*` → `GtkWidget*` without a per-widget dynamic_cast branch.
GtkWidget* dispatch_toolbar(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::toolbar*>(v); w && w->has_handler()) {
        return static_cast<GtkWidget*>(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_toolbar);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
