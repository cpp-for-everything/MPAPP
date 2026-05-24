// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 switch handler implementation.

#include "mpapp/handlers/linux/switch_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp::internal {

namespace {

struct state_set_ctx {
    basic_switch_*                          target;
    switch_handler<platform::linux_>* handler;
};

gboolean on_state_set(GtkSwitch* /*sw*/, gboolean state, gpointer user_data) {
    auto* ctx = static_cast<state_set_ctx*>(user_data);
    if (ctx == nullptr || ctx->target == nullptr || ctx->handler == nullptr) return FALSE;
    if (ctx->handler->native() == nullptr) return FALSE;

    // Mirror user-driven toggle into the cross-platform Observable.
    // Return TRUE to indicate we've handled the state-set (and prevent
    // the default handler from re-toggling).
    const bool v = state == TRUE;
    if (ctx->target->is_on.get() != v) {
        ctx->target->is_on.set(v);
    }
    // Sync the visible state to match (the default would also do this,
    // but explicitly call to short-circuit the default).
    gtk_switch_set_state(GTK_SWITCH(static_cast<GtkWidget*>(ctx->handler->native())),
                         state);
    return TRUE;
}

} // namespace

switch_handler<platform::linux_>::switch_handler() {
    native_ = gtk_switch_new();
}

switch_handler<platform::linux_>::~switch_handler() {
    if (native_ != nullptr && state_set_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    state_set_handler_id_);
        state_set_handler_id_ = 0;
    }
}

void switch_handler<platform::linux_>::apply_is_on(bool on) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    gtk_switch_set_active(GTK_SWITCH(static_cast<GtkWidget*>(native_)),
                          on ? TRUE : FALSE);
    suppress_echo_ = false;
}

void switch_handler<platform::linux_>::map_is_on(basic_switch_& s) {
    apply_is_on(s.is_on.get());
    s.is_on.changed.subscribe(is_on_slot_, is_on_cb_);

    if (native_ == nullptr) return;
    auto* ctx = new state_set_ctx{&s, this};
    if (state_set_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    state_set_handler_id_);
    }
    state_set_handler_id_ = g_signal_connect_data(
        static_cast<GtkWidget*>(native_),
        "state-set",
        G_CALLBACK(on_state_set),
        ctx,
        +[](gpointer p, GClosure*) { delete static_cast<state_set_ctx*>(p); },
        static_cast<GConnectFlags>(0));
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_switch_ so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/internal/basic_switch_.hpp"

namespace {

GtkWidget* dispatch_switch(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_switch_*>(v); w && w->has_handler()) {
        return GTK_WIDGET(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_switch); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
