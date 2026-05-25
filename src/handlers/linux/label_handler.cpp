// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — GTK4 basic_label handler implementation.

#include "mpapp/handlers/linux/label_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp::internal {

label_handler<platform::linux_>::label_handler() {
    native_ = gtk_label_new("");
}

label_handler<platform::linux_>::~label_handler() = default;

void label_handler<platform::linux_>::apply_text(const std::string& text) {
    if (native_ != nullptr) {
        gtk_label_set_text(GTK_LABEL(static_cast<GtkWidget*>(native_)),
                           text.c_str());
    }
}

void label_handler<platform::linux_>::map_text(basic_label& l) {
    apply_text(l.text.get());
    l.text.changed.subscribe(text_slot_, text_cb_);
}

void label_handler<platform::linux_>::map_gestures(basic_label& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_label so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/internal/basic_label.hpp"

namespace {

GtkWidget* dispatch_label(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_label*>(v); w && w->has_handler()) {
        return GTK_WIDGET(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_label); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
