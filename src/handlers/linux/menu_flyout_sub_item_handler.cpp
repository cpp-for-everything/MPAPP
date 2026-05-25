// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_menu_flyout_sub_item handler implementation.

#include "mpapp/handlers/linux/menu_flyout_sub_item_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

#include "mpapp/internal/basic_menu_flyout_sub_item.hpp"
#include "mpapp/view.hpp"

namespace mpapp::internal {

menu_flyout_sub_item_handler<platform::linux_>::menu_flyout_sub_item_handler() {
    // Sub-item presents as a basic_button that, when clicked, opens its own
    // nested popover. GtkMenuButton handles the popover toggle.
    GtkWidget* btn        = gtk_menu_button_new();
    GtkWidget* sub_pop    = gtk_popover_new();
    GtkWidget* sub_box    = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_popover_set_child(GTK_POPOVER(sub_pop), sub_box);
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(btn), sub_pop);
    gtk_widget_add_css_class(btn, "flat");
    native_      = btn;
    sub_popover_ = sub_pop;
    sub_box_     = sub_box;
}

menu_flyout_sub_item_handler<platform::linux_>::~menu_flyout_sub_item_handler() = default;

void menu_flyout_sub_item_handler<platform::linux_>::apply_text(const std::string& v) {
    if (native_ == nullptr) return;
    gtk_menu_button_set_label(GTK_MENU_BUTTON(static_cast<GtkWidget*>(native_)), v.c_str());
}

void menu_flyout_sub_item_handler<platform::linux_>::apply_items(const std::vector<view*>& v) {
    if (sub_box_ == nullptr) return;
    GtkBox* box = GTK_BOX(static_cast<GtkWidget*>(sub_box_));
    while (GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(box))) {
        gtk_box_remove(box, child);
    }
    for (view* child : v) {
        if (child == nullptr) continue;
        if (GtkWidget* w = detail::linux_dispatch::dispatch(child); w != nullptr) {
            gtk_box_append(box, w);
        }
    }
}

void menu_flyout_sub_item_handler<platform::linux_>::map_text(basic_menu_flyout_sub_item& s) {
    apply_text(s.text.get());
    s.text.changed.subscribe(text_slot_, text_cb_);
}

void menu_flyout_sub_item_handler<platform::linux_>::map_items(basic_menu_flyout_sub_item& s) {
    apply_items(s.items.get());
    s.items.changed.subscribe(items_slot_, items_cb_);
}

void menu_flyout_sub_item_handler<platform::linux_>::map_gestures(basic_menu_flyout_sub_item& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --

namespace {

GtkWidget* dispatch_menu_flyout_sub_item(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::internal::basic_menu_flyout_sub_item*>(v); s && s->has_handler()) {
        return GTK_WIDGET(static_cast<GtkWidget*>(s->handler().native()));
    }
    return nullptr;
}

struct registrar_mfsi {
    registrar_mfsi() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_menu_flyout_sub_item); }
};

[[maybe_unused]] registrar_mfsi _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
