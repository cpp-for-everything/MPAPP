// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_menu_bar_item handler implementation.

#include "mpapp/handlers/linux/menu_bar_item_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

menu_bar_item_handler<platform::linux_>::menu_bar_item_handler() {
    // GtkMenuButton is the closest GTK4 primitive for a labelled menu
    // basic_entry that can host a popover — when basic_menu_flyout lands the
    // popover gets attached via `gtk_menu_button_set_popover()`.
    GtkWidget* btn = gtk_menu_button_new();
    // Show only the basic_label slot; the dropdown arrow is GTK4 default.
    gtk_menu_button_set_label(GTK_MENU_BUTTON(btn), "");
    native_ = btn;
}

menu_bar_item_handler<platform::linux_>::~menu_bar_item_handler() = default;

void menu_bar_item_handler<platform::linux_>::apply_title(const std::string& v) {
    if (native_ == nullptr) return;
    gtk_menu_button_set_label(GTK_MENU_BUTTON(static_cast<GtkWidget*>(native_)),
                              v.c_str());
}

void menu_bar_item_handler<platform::linux_>::apply_items(const std::vector<view*>& /*v*/) {
    // Children land alongside basic_menu_flyout (M-04c). For the M-04b
    // baseline the count is observed at the mock-handler level; the
    // real handler is a no-op until a popover model is wired through.
    if (native_ == nullptr) return;
}

void menu_bar_item_handler<platform::linux_>::map_title(basic_menu_bar_item& m) {
    apply_title(m.title.get());
    m.title.changed.subscribe(title_slot_, title_cb_);
}

void menu_bar_item_handler<platform::linux_>::map_items(basic_menu_bar_item& m) {
    apply_items(m.items.get());
    m.items.changed.subscribe(items_slot_, items_cb_);
}

} // namespace mpapp::internal
// --- ADR-0013 self-registration --------------------------------------------

namespace {

GtkWidget* dispatch_menu_bar_item(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_menu_bar_item*>(v); w && w->has_handler()) {
        return static_cast<GtkWidget*>(w->handler().native());
    }
    return nullptr;
}

struct registrar_menu_bar_item {
    registrar_menu_bar_item() {
        ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_menu_bar_item);
    }
};

[[maybe_unused]] registrar_menu_bar_item _reg_mbi;

} // namespace

#endif // __linux__ && !__ANDROID__
