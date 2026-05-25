// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_search_bar handler implementation.

#include "mpapp/handlers/linux/search_bar_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp::internal {

search_bar_handler<platform::linux_>::search_bar_handler() {
    native_ = gtk_search_entry_new();
}

search_bar_handler<platform::linux_>::~search_bar_handler() = default;

void search_bar_handler<platform::linux_>::apply_text(const std::string& v) {
    if (native_ == nullptr || suppress_echo_) return;
    suppress_echo_ = true;
    gtk_editable_set_text(GTK_EDITABLE(static_cast<GtkWidget*>(native_)), v.c_str());
    suppress_echo_ = false;
}

void search_bar_handler<platform::linux_>::apply_placeholder(const std::string& v) {
    if (native_ == nullptr) return;
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(static_cast<GtkWidget*>(native_)), v.c_str());
}

void search_bar_handler<platform::linux_>::map_text(basic_search_bar& s) {
    apply_text(s.text.get());
    s.text.changed.subscribe(text_slot_, text_cb_);
}
void search_bar_handler<platform::linux_>::map_placeholder(basic_search_bar& s) {
    apply_placeholder(s.placeholder.get());
    s.placeholder.changed.subscribe(placeholder_slot_, placeholder_cb_);
}

void search_bar_handler<platform::linux_>::map_gestures(basic_search_bar& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_search_bar so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/internal/basic_search_bar.hpp"

namespace {

GtkWidget* dispatch_search_bar(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_search_bar*>(v); w && w->has_handler()) {
        return GTK_WIDGET(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_search_bar); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
