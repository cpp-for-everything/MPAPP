// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_check_box handler implementation.

#include "mpapp/handlers/linux/check_box_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp::internal {

namespace {

struct toggled_ctx {
    basic_check_box*                          target;
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

void check_box_handler<platform::linux_>::map_is_checked(basic_check_box& c) {
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

void check_box_handler<platform::linux_>::map_gestures(basic_check_box& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_check_box so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/internal/basic_check_box.hpp"

namespace {

GtkWidget* dispatch_check_box(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_check_box*>(v); w && w->has_handler()) {
        return GTK_WIDGET(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_check_box); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
