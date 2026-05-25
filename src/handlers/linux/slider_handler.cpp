// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_slider handler implementation.

#include "mpapp/handlers/linux/slider_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp::internal {

namespace {

struct value_changed_ctx {
    basic_slider*                            target;
    slider_handler<platform::linux_>*  handler;
};

void on_value_changed(GtkRange* range, gpointer user_data) {
    auto* ctx = static_cast<value_changed_ctx*>(user_data);
    if (ctx == nullptr || ctx->target == nullptr || ctx->handler == nullptr) return;
    const double v = gtk_range_get_value(range);
    if (ctx->target->value.get() != v) {
        ctx->target->value.set(v);
    }
}

} // namespace

slider_handler<platform::linux_>::slider_handler() {
    native_ = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 1.0, 0.01);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(native_), TRUE);
}

slider_handler<platform::linux_>::~slider_handler() {
    if (native_ != nullptr && value_changed_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    value_changed_handler_id_);
        value_changed_handler_id_ = 0;
    }
}

void slider_handler<platform::linux_>::apply_value(double v) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    gtk_range_set_value(GTK_RANGE(static_cast<GtkWidget*>(native_)), v);
    suppress_echo_ = false;
}

void slider_handler<platform::linux_>::apply_minimum(double v) {
    if (native_ == nullptr) return;
    GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(static_cast<GtkWidget*>(native_)));
    gtk_adjustment_set_lower(adj, v);
}

void slider_handler<platform::linux_>::apply_maximum(double v) {
    if (native_ == nullptr) return;
    GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(static_cast<GtkWidget*>(native_)));
    gtk_adjustment_set_upper(adj, v);
}

void slider_handler<platform::linux_>::map_value(basic_slider& s) {
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

void slider_handler<platform::linux_>::map_minimum(basic_slider& s) {
    apply_minimum(s.minimum.get());
    s.minimum.changed.subscribe(minimum_slot_, minimum_cb_);
}

void slider_handler<platform::linux_>::map_maximum(basic_slider& s) {
    apply_maximum(s.maximum.get());
    s.maximum.changed.subscribe(maximum_slot_, maximum_cb_);
}

void slider_handler<platform::linux_>::map_gestures(basic_slider& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_slider so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/internal/basic_slider.hpp"

namespace {

GtkWidget* dispatch_slider(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_slider*>(v); w && w->has_handler()) {
        return GTK_WIDGET(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_slider); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
