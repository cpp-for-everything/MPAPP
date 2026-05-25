// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_menu_flyout_item handler implementation.

#include "mpapp/handlers/linux/menu_flyout_item_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

#include "mpapp/internal/basic_menu_flyout_item.hpp"

namespace mpapp::internal {

namespace {

extern "C" void mpapp_mfi_on_clicked(GtkButton* /*btn*/, gpointer user_data) {
    auto* owner = static_cast<basic_menu_flyout_item*>(user_data);
    if (owner != nullptr) {
        owner->clicked.emit();
    }
}

} // namespace

menu_flyout_item_handler<platform::linux_>::menu_flyout_item_handler() {
    native_ = gtk_button_new_with_label("");
    // Match the visual style of a menu item rather than a regular
    // chrome-heavy basic_button — flat removes the basic_button's default
    // background so the popover looks like a menu.
    gtk_widget_add_css_class(GTK_WIDGET(static_cast<GtkWidget*>(native_)), "flat");
}

menu_flyout_item_handler<platform::linux_>::~menu_flyout_item_handler() {
    if (native_ != nullptr && click_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkButton*>(native_), click_id_);
        click_id_ = 0;
    }
}

void menu_flyout_item_handler<platform::linux_>::apply_text(const std::string& v) {
    if (native_ == nullptr) return;
    gtk_button_set_label(GTK_BUTTON(static_cast<GtkWidget*>(native_)), v.c_str());
}

void menu_flyout_item_handler<platform::linux_>::apply_is_enabled(bool v) {
    if (native_ == nullptr) return;
    gtk_widget_set_sensitive(GTK_WIDGET(static_cast<GtkWidget*>(native_)), v ? TRUE : FALSE);
}

void menu_flyout_item_handler<platform::linux_>::map_text(basic_menu_flyout_item& i) {
    owner_ = &i;
    apply_text(i.text.get());
    i.text.changed.subscribe(text_slot_, text_cb_);
    // Wire the native click → cross-platform `clicked`. Idempotent.
    if (native_ != nullptr && click_id_ == 0) {
        click_id_ = g_signal_connect(
            G_OBJECT(static_cast<GtkButton*>(native_)),
            "clicked",
            G_CALLBACK(mpapp_mfi_on_clicked),
            owner_);
    }
}

void menu_flyout_item_handler<platform::linux_>::map_is_enabled(basic_menu_flyout_item& i) {
    apply_is_enabled(i.is_enabled.get());
    i.is_enabled.changed.subscribe(is_enabled_slot_, is_enabled_cb_);
}

void menu_flyout_item_handler<platform::linux_>::map_gestures(basic_menu_flyout_item& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --

namespace {

GtkWidget* dispatch_menu_flyout_item(::mpapp::view* v) {
    if (auto* i = dynamic_cast<::mpapp::internal::basic_menu_flyout_item*>(v); i && i->has_handler()) {
        return GTK_WIDGET(static_cast<GtkWidget*>(i->handler().native()));
    }
    return nullptr;
}

struct registrar_mfi {
    registrar_mfi() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_menu_flyout_item); }
};

[[maybe_unused]] registrar_mfi _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
