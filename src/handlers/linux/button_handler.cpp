// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — GTK4 basic_button handler implementation.

#include "mpapp/handlers/linux/button_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/gesture_attach.hpp"

namespace mpapp::internal {

namespace {

void on_clicked(GtkButton* /*btn*/, gpointer user_data) {
    auto* b = static_cast<basic_button*>(user_data);
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
    // basic_button pointer.
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

void button_handler<platform::linux_>::map_text(basic_button& b) {
    bound_ = &b;
    apply_text(b.text.get());
    b.text.changed.subscribe(text_slot_, text_cb_);
}

void button_handler<platform::linux_>::apply_semantics(const std::string& desc) {
    if (native_ == nullptr || desc.empty()) return;
    gtk_accessible_update_property(
        GTK_ACCESSIBLE(static_cast<GtkWidget*>(native_)),
        GTK_ACCESSIBLE_PROPERTY_LABEL, desc.c_str(), -1);
}

void button_handler<platform::linux_>::map_semantics(basic_button& b) {
    apply_semantics(b.semantic_description.get());
    b.semantic_description.changed.subscribe(sem_slot_, sem_cb_);
}

void button_handler<platform::linux_>::map_gestures(basic_button& b) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), b);
}

void button_handler<platform::linux_>::map_clicked(basic_button& b) {
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

} // namespace mpapp::internal

// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_button so ADR-0013 fall-through
// dispatch can find its GtkWidget* without the legacy dynamic_cast chain.

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/internal/basic_button.hpp"

namespace {

GtkWidget* dispatch_button(::mpapp::view* v) {
    if (auto* b = dynamic_cast<::mpapp::internal::basic_button*>(v); b && b->has_handler()) {
        return GTK_WIDGET(b->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_button); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
