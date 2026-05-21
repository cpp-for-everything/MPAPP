// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 menu_flyout handler implementation.

#include "mpapp/handlers/linux/menu_flyout_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

#include "mpapp/menu_flyout.hpp"
#include "mpapp/view.hpp"

namespace mpapp {

menu_flyout_handler<platform::linux_>::menu_flyout_handler() {
    GtkWidget* popover = gtk_popover_new();
    GtkWidget* box     = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_popover_set_child(GTK_POPOVER(popover), box);
    native_ = popover;
    box_    = box;
}

menu_flyout_handler<platform::linux_>::~menu_flyout_handler() = default;

void menu_flyout_handler<platform::linux_>::apply_items(const std::vector<view*>& v) {
    if (box_ == nullptr) return;
    GtkBox* box = GTK_BOX(static_cast<GtkWidget*>(box_));
    // Drain existing children.
    while (GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(box))) {
        gtk_box_remove(box, child);
    }
    for (view* child : v) {
        if (child == nullptr) continue;
        // ADR-0013 — each menu_flyout_item / _separator / _sub_item
        // self-registers a dispatcher that returns its GtkWidget*.
        if (GtkWidget* w = detail::linux_dispatch::dispatch(child); w != nullptr) {
            gtk_box_append(box, w);
        }
    }
}

void menu_flyout_handler<platform::linux_>::apply_is_open(bool v) {
    if (native_ == nullptr) return;
    GtkPopover* popover = GTK_POPOVER(static_cast<GtkWidget*>(native_));
    if (v) {
        gtk_popover_popup(popover);
    } else {
        gtk_popover_popdown(popover);
    }
}

void menu_flyout_handler<platform::linux_>::map_items(menu_flyout& f) {
    apply_items(f.items.get());
    f.items.changed.subscribe(items_slot_, items_cb_);
}

void menu_flyout_handler<platform::linux_>::map_is_open(menu_flyout& f) {
    apply_is_open(f.is_open.get());
    f.is_open.changed.subscribe(is_open_slot_, is_open_cb_);
}

} // namespace mpapp

// ---------- Self-registration with the per-platform dispatch registry --

namespace {

// menu_flyout is a popup surface (`GtkPopover`), not a child of a
// regular GtkBox/GtkGrid container. Returning nullptr ensures
// container dispatch sites skip it cleanly. The registrar is still
// installed for ADR-0013 uniformity.
GtkWidget* dispatch_menu_flyout(::mpapp::view* /*v*/) {
    return nullptr;
}

struct registrar_mf {
    registrar_mf() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_menu_flyout); }
};

[[maybe_unused]] registrar_mf _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
