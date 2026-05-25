// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_stepper handler implementation.

#include "mpapp/handlers/linux/stepper_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp::internal {

namespace {

struct value_changed_ctx {
    basic_stepper*                            target;
    stepper_handler<platform::linux_>*  handler;
};

void on_value_changed(GtkSpinButton* btn, gpointer user_data) {
    auto* ctx = static_cast<value_changed_ctx*>(user_data);
    if (ctx == nullptr || ctx->target == nullptr) return;
    const double v = gtk_spin_button_get_value(btn);
    if (ctx->target->value.get() != v) {
        ctx->target->value.set(v);
    }
}

} // namespace

stepper_handler<platform::linux_>::stepper_handler() {
    native_ = gtk_spin_button_new_with_range(0.0, 100.0, 1.0);
}

stepper_handler<platform::linux_>::~stepper_handler() {
    if (native_ != nullptr && value_changed_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    value_changed_handler_id_);
        value_changed_handler_id_ = 0;
    }
}

void stepper_handler<platform::linux_>::apply_value(double v) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(static_cast<GtkWidget*>(native_)), v);
    suppress_echo_ = false;
}

void stepper_handler<platform::linux_>::apply_minimum(double v) {
    if (native_ == nullptr) return;
    double lo = 0.0, hi = 0.0;
    gtk_spin_button_get_range(GTK_SPIN_BUTTON(static_cast<GtkWidget*>(native_)), &lo, &hi);
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(static_cast<GtkWidget*>(native_)), v, hi);
}

void stepper_handler<platform::linux_>::apply_maximum(double v) {
    if (native_ == nullptr) return;
    double lo = 0.0, hi = 0.0;
    gtk_spin_button_get_range(GTK_SPIN_BUTTON(static_cast<GtkWidget*>(native_)), &lo, &hi);
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(static_cast<GtkWidget*>(native_)), lo, v);
}

void stepper_handler<platform::linux_>::apply_interval(double v) {
    if (native_ == nullptr) return;
    double step = 0.0, basic_page = 0.0;
    gtk_spin_button_get_increments(GTK_SPIN_BUTTON(static_cast<GtkWidget*>(native_)), &step, &basic_page);
    gtk_spin_button_set_increments(GTK_SPIN_BUTTON(static_cast<GtkWidget*>(native_)), v, basic_page > 0 ? basic_page : v * 10.0);
}

void stepper_handler<platform::linux_>::map_value(basic_stepper& s) {
    apply_value(s.value.get());
    s.value.changed.subscribe(value_slot_, value_cb_);

    if (native_ == nullptr) return;
    auto* ctx = new value_changed_ctx{&s, this};
    if (value_changed_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    value_changed_handler_id_);
    }
    value_changed_handler_id_ = g_signal_connect_data(
        static_cast<GtkWidget*>(native_),
        "value-changed",
        G_CALLBACK(on_value_changed),
        ctx,
        +[](gpointer p, GClosure*) { delete static_cast<value_changed_ctx*>(p); },
        static_cast<GConnectFlags>(0));
}

void stepper_handler<platform::linux_>::map_minimum(basic_stepper& s) {
    apply_minimum(s.minimum.get());
    s.minimum.changed.subscribe(minimum_slot_, minimum_cb_);
}
void stepper_handler<platform::linux_>::map_maximum(basic_stepper& s) {
    apply_maximum(s.maximum.get());
    s.maximum.changed.subscribe(maximum_slot_, maximum_cb_);
}
void stepper_handler<platform::linux_>::map_interval(basic_stepper& s) {
    apply_interval(s.interval.get());
    s.interval.changed.subscribe(interval_slot_, interval_cb_);
}

void stepper_handler<platform::linux_>::map_gestures(basic_stepper& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_stepper so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/internal/basic_stepper.hpp"

namespace {

GtkWidget* dispatch_stepper(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_stepper*>(v); w && w->has_handler()) {
        return GTK_WIDGET(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_stepper); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
