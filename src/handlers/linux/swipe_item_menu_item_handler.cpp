// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 swipe_item_menu_item handler implementation.

#include "mpapp/handlers/linux/swipe_item_menu_item_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp {

namespace {

void on_swipe_item_clicked(GtkButton* /*btn*/, gpointer user_data) {
    auto* sig = static_cast<mpapp::signal<>*>(user_data);
    if (sig != nullptr) sig->emit();
}

} // namespace

swipe_item_menu_item_handler<platform::linux_>::swipe_item_menu_item_handler() {
    GtkWidget* btn = gtk_button_new_with_label("");
    native_ = btn;
}

swipe_item_menu_item_handler<platform::linux_>::~swipe_item_menu_item_handler() {
    if (native_ != nullptr && clicked_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    static_cast<gulong>(clicked_handler_id_));
        clicked_handler_id_ = 0;
    }
}

void swipe_item_menu_item_handler<platform::linux_>::apply_text(const std::string& v) {
    if (native_ == nullptr) return;
    gtk_button_set_label(GTK_BUTTON(static_cast<GtkWidget*>(native_)), v.c_str());
}

void swipe_item_menu_item_handler<platform::linux_>::apply_icon_uri(const std::string& v) {
    if (native_ == nullptr) return;
    if (v.empty()) {
        gtk_button_set_icon_name(GTK_BUTTON(static_cast<GtkWidget*>(native_)), nullptr);
        return;
    }
    // Treat the URI as a theme icon name on this path (the richer
    // file-image + bitmap-decode plumbing lands with image-source).
    gtk_button_set_icon_name(GTK_BUTTON(static_cast<GtkWidget*>(native_)), v.c_str());
}

void swipe_item_menu_item_handler<platform::linux_>::map_text(swipe_item_menu_item& m) {
    apply_text(m.text.get());
    m.text.changed.subscribe(text_slot_, text_cb_);
}

void swipe_item_menu_item_handler<platform::linux_>::map_icon_uri(swipe_item_menu_item& m) {
    apply_icon_uri(m.icon_uri.get());
    m.icon_uri.changed.subscribe(icon_slot_, icon_cb_);
}

void swipe_item_menu_item_handler<platform::linux_>::map_invoked(swipe_item_menu_item& m) {
    if (native_ == nullptr) return;
    invoked_signal_ = &m.invoked;
    if (clicked_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    static_cast<gulong>(clicked_handler_id_));
        clicked_handler_id_ = 0;
    }
    clicked_handler_id_ = static_cast<unsigned long>(g_signal_connect(
        static_cast<GtkWidget*>(native_),
        "clicked",
        G_CALLBACK(on_swipe_item_clicked),
        invoked_signal_));
}

} // namespace mpapp

// ----- ADR-0013 self-registration --------------------------------------

namespace {

GtkWidget* dispatch_swipe_item_menu_item(::mpapp::view* v) {
    if (auto* m = dynamic_cast<::mpapp::swipe_item_menu_item*>(v); m && m->has_handler()) {
        return GTK_WIDGET(m->handler().native());
    }
    return nullptr;
}

struct swipe_item_menu_item_registrar {
    swipe_item_menu_item_registrar() {
        ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_swipe_item_menu_item);
    }
};

[[maybe_unused]] swipe_item_menu_item_registrar _swipe_item_menu_item_reg;

} // namespace

#endif // __linux__ && !__ANDROID__
