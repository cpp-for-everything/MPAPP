// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — GTK4 button handler implementation.

#include "mpapp/handlers/linux/button_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp {

namespace {

void on_clicked(GtkButton* /*btn*/, gpointer user_data) {
    auto* b = static_cast<mpapp::button*>(user_data);
    if (b != nullptr) {
        b->clicked.emit();
    }
}

} // namespace

button_handler<platform::linux_>::button_handler() {
    native_ = gtk_button_new_with_label("");
}

button_handler<platform::linux_>::~button_handler() {
    // The GtkButton is parented into a GtkBox/GtkWindow by the time
    // we're done; that owner unrefs it. Disconnect the clicked handler
    // explicitly so a late "clicked" signal doesn't fire into a freed
    // button pointer.
    if (native_ != nullptr && click_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    click_handler_id_);
        click_handler_id_ = 0;
    }
}

void button_handler<platform::linux_>::apply_text(const std::string& text) {
    if (native_ != nullptr) {
        gtk_button_set_label(GTK_BUTTON(static_cast<GtkWidget*>(native_)),
                             text.c_str());
    }
}

void button_handler<platform::linux_>::map_text(button& b) {
    bound_ = &b;
    apply_text(b.text.get());
    b.text.changed.subscribe(text_slot_, text_cb_);
}

void button_handler<platform::linux_>::map_clicked(button& b) {
    bound_ = &b;
    if (native_ == nullptr) {
        return;
    }
    if (click_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    click_handler_id_);
    }
    click_handler_id_ = g_signal_connect(
        static_cast<GtkWidget*>(native_),
        "clicked",
        G_CALLBACK(on_clicked),
        &b);
}

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
