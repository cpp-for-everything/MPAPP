// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_templated_view handler implementation.

#include "mpapp/handlers/linux/templated_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

#include "mpapp/internal/basic_templated_view.hpp"
#include "mpapp/view.hpp"

namespace mpapp::internal {

templated_view_handler<platform::linux_>::templated_view_handler() {
    // Horizontal `GtkBox` used as a single-child host (GTK4 dropped
    // `GtkBin`; the box mirrors the shape `basic_content_view` uses).
    native_ = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
}

templated_view_handler<platform::linux_>::~templated_view_handler() = default;

void templated_view_handler<platform::linux_>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    GtkBox* box = GTK_BOX(static_cast<GtkWidget*>(native_));
    if (current_child_ != nullptr) {
        gtk_box_remove(box, GTK_WIDGET(current_child_));
        current_child_ = nullptr;
    }
    if (!v) return;
    // ADR-0013 registry first; if no widget is registered for the child
    // type, leave content empty (legacy widgets that haven't migrated
    // yet are rendered by their parent's existing dispatch chain only —
    // basic_templated_view is brand-new and goes through the registry only).
    if (GtkWidget* child = detail::linux_dispatch::dispatch(v.get()); child != nullptr) {
        gtk_box_append(box, child);
        current_child_ = child;
    }
}

void templated_view_handler<platform::linux_>::apply_template_id(const std::string& v) {
    // P3 templating engine is deferred — record the id for later wiring.
    template_id_ = v;
}

void templated_view_handler<platform::linux_>::map_content(basic_templated_view& t) {
    apply_content(t.content.get());
    t.content.changed.subscribe(content_slot_, content_cb_);
}

void templated_view_handler<platform::linux_>::map_template_id(basic_templated_view& t) {
    apply_template_id(t.template_id.get());
    t.template_id.changed.subscribe(template_id_slot_, template_id_cb_);
}

void templated_view_handler<platform::linux_>::bind_content(basic_templated_view& t, view& child) {
    t.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --

namespace {

GtkWidget* dispatch_templated_view(::mpapp::view* v) {
    if (auto* t = dynamic_cast<::mpapp::internal::basic_templated_view*>(v); t && t->has_handler()) {
        return GTK_WIDGET(t->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_templated_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
