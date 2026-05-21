// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 menu_bar handler implementation.

#include "mpapp/handlers/linux/menu_bar_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <vector>

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/handlers/linux/menu_bar_item_handler.hpp"
#include "mpapp/menu_bar_item.hpp"

namespace mpapp {

namespace {

// Remove every direct child of the menubar box. GTK4 doesn't ship a
// `gtk_box_remove_all`, so we walk the linked list explicitly.
void box_remove_all(GtkBox* box) {
    GtkWidget* w = gtk_widget_get_first_child(GTK_WIDGET(box));
    while (w != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(w);
        gtk_box_remove(box, w);
        w = next;
    }
}

} // namespace

menu_bar_handler<platform::linux_>::menu_bar_handler() {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(box, "menubar");
    native_ = box;
}

menu_bar_handler<platform::linux_>::~menu_bar_handler() = default;

void menu_bar_handler<platform::linux_>::apply_items(const std::vector<view*>& v) {
    if (native_ == nullptr) return;
    GtkBox* box = GTK_BOX(static_cast<GtkWidget*>(native_));
    box_remove_all(box);
    for (view* child : v) {
        if (child == nullptr) continue;
        // Resolve via direct dynamic_cast to the strongly-typed
        // menu_bar_item — the ADR-0013 registry would also work, but
        // calling it here would force every other widget's dispatcher
        // to run during a hot rebuild. The strongly-typed cast is O(1).
        if (auto* mbi = dynamic_cast<menu_bar_item*>(child); mbi && mbi->has_handler()) {
            if (GtkWidget* w = static_cast<GtkWidget*>(mbi->handler().native()); w != nullptr) {
                gtk_box_append(box, w);
            }
        }
    }
}

void menu_bar_handler<platform::linux_>::map_items(menu_bar& b) {
    apply_items(b.items.get());
    b.items.changed.subscribe(items_slot_, items_cb_);
}

} // namespace mpapp

// --- ADR-0013 self-registration --------------------------------------------

namespace {

GtkWidget* dispatch_menu_bar(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::menu_bar*>(v); w && w->has_handler()) {
        return static_cast<GtkWidget*>(w->handler().native());
    }
    return nullptr;
}

struct registrar_menu_bar {
    registrar_menu_bar() {
        ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_menu_bar);
    }
};

[[maybe_unused]] registrar_menu_bar _reg_mb;

} // namespace

#endif // __linux__ && !__ANDROID__
