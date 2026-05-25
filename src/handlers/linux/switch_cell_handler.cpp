// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_switch_cell handler implementation.

#include "mpapp/handlers/linux/switch_cell_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

struct state_set_ctx {
    basic_switch_cell*                          target;
    switch_cell_handler<platform::linux_>* handler;
};

gboolean on_state_set(GtkSwitch* /*sw*/, gboolean state, gpointer user_data) {
    auto* ctx = static_cast<state_set_ctx*>(user_data);
    if (ctx == nullptr || ctx->target == nullptr || ctx->handler == nullptr) return FALSE;
    if (ctx->handler->native() == nullptr) return FALSE;

    const bool v = state == TRUE;
    if (ctx->target->on.get() != v) {
        ctx->target->on.set(v);
    }
    ctx->target->on_changed.emit(v);

    // Mirror the visible state to match the requested state; otherwise
    // the default handler will also try to set it, which is harmless,
    // but doing it here avoids a small flicker on slow paths.
    GtkWidget* sw_widget = GTK_WIDGET(g_object_get_data(
        G_OBJECT(ctx->handler->native()), "mpapp_switch"));
    if (sw_widget != nullptr) {
        gtk_switch_set_state(GTK_SWITCH(sw_widget), state);
    }
    return TRUE;
}

} // namespace

switch_cell_handler<platform::linux_>::switch_cell_handler() {
    native_   = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    label_    = gtk_label_new("");
    switch_w_ = gtk_switch_new();

    gtk_widget_set_halign(static_cast<GtkWidget*>(label_), GTK_ALIGN_START);
    gtk_widget_set_valign(static_cast<GtkWidget*>(label_), GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(label_), TRUE);

    gtk_widget_set_halign(static_cast<GtkWidget*>(switch_w_), GTK_ALIGN_END);
    gtk_widget_set_valign(static_cast<GtkWidget*>(switch_w_), GTK_ALIGN_CENTER);

    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(native_)),
                   static_cast<GtkWidget*>(label_));
    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(native_)),
                   static_cast<GtkWidget*>(switch_w_));

    // Stash the switch widget on the box so the state-set callback can
    // find it without dragging this handler instance into the closure.
    g_object_set_data(G_OBJECT(native_), "mpapp_switch", switch_w_);

    // Native row padding.
    gtk_widget_set_margin_start (static_cast<GtkWidget*>(native_), 12);
    gtk_widget_set_margin_end   (static_cast<GtkWidget*>(native_), 12);
    gtk_widget_set_margin_top   (static_cast<GtkWidget*>(native_), 6);
    gtk_widget_set_margin_bottom(static_cast<GtkWidget*>(native_), 6);
}

switch_cell_handler<platform::linux_>::~switch_cell_handler() {
    if (switch_w_ != nullptr && state_set_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(switch_w_),
                                    state_set_handler_id_);
        state_set_handler_id_ = 0;
    }
}

void switch_cell_handler<platform::linux_>::apply_text(const std::string& v) {
    if (label_ == nullptr) return;
    gtk_label_set_text(GTK_LABEL(static_cast<GtkWidget*>(label_)), v.c_str());
}

void switch_cell_handler<platform::linux_>::apply_on(bool v) {
    if (switch_w_ == nullptr) return;
    suppress_echo_ = true;
    gtk_switch_set_active(GTK_SWITCH(static_cast<GtkWidget*>(switch_w_)),
                          v ? TRUE : FALSE);
    suppress_echo_ = false;
}

void switch_cell_handler<platform::linux_>::map_text(basic_switch_cell& c) {
    apply_text(c.text.get());
    c.text.changed.subscribe(text_slot_, text_cb_);
}

void switch_cell_handler<platform::linux_>::map_on(basic_switch_cell& c) {
    bound_ = &c;
    apply_on(c.on.get());
    c.on.changed.subscribe(on_slot_, on_cb_);

    if (switch_w_ == nullptr) return;
    if (state_set_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(switch_w_),
                                    state_set_handler_id_);
        state_set_handler_id_ = 0;
    }
    auto* ctx = new state_set_ctx{&c, this};
    state_set_handler_id_ = g_signal_connect_data(
        static_cast<GtkWidget*>(switch_w_),
        "state-set",
        G_CALLBACK(on_state_set),
        ctx,
        +[](gpointer p, GClosure*) { delete static_cast<state_set_ctx*>(p); },
        static_cast<GConnectFlags>(0));
}

void switch_cell_handler<platform::linux_>::map_gestures(basic_switch_cell& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_switch_cell(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::internal::basic_switch_cell*>(v); c && c->has_sc_handler()) {
        return GTK_WIDGET(c->sc_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_switch_cell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
