// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 check_box handler implementation.

#include "mpapp/handlers/linux/check_box_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp {

namespace {

struct toggled_ctx {
    check_box*                          target;
    check_box_handler<platform::linux_>* handler;
};

void on_toggled(GtkCheckButton* btn, gpointer user_data) {
    auto* ctx = static_cast<toggled_ctx*>(user_data);
    if (ctx == nullptr || ctx->target == nullptr || ctx->handler == nullptr) return;
    const bool v = gtk_check_button_get_active(btn) == TRUE;
    if (ctx->target->is_checked.get() != v) {
        ctx->target->is_checked.set(v);
    }
}

} // namespace

check_box_handler<platform::linux_>::check_box_handler() {
    native_ = gtk_check_button_new();
}

check_box_handler<platform::linux_>::~check_box_handler() {
    if (native_ != nullptr && toggled_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    toggled_handler_id_);
        toggled_handler_id_ = 0;
    }
}

void check_box_handler<platform::linux_>::apply_is_checked(bool v) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    gtk_check_button_set_active(GTK_CHECK_BUTTON(static_cast<GtkWidget*>(native_)),
                                v ? TRUE : FALSE);
    suppress_echo_ = false;
}

void check_box_handler<platform::linux_>::map_is_checked(check_box& c) {
    apply_is_checked(c.is_checked.get());
    c.is_checked.changed.subscribe(slot_, cb_);

    if (native_ == nullptr) return;
    auto* ctx = new toggled_ctx{&c, this};
    if (toggled_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    toggled_handler_id_);
    }
    toggled_handler_id_ = g_signal_connect_data(
        static_cast<GtkWidget*>(native_),
        "toggled",
        G_CALLBACK(on_toggled),
        ctx,
        +[](gpointer p, GClosure*) { delete static_cast<toggled_ctx*>(p); },
        static_cast<GConnectFlags>(0));
}

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
