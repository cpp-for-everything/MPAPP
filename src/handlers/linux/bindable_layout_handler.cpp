// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 bindable_layout handler implementation.

#include "mpapp/handlers/linux/bindable_layout_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp {

bindable_layout_handler<platform::linux_>::bindable_layout_handler() {
    native_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
}

bindable_layout_handler<platform::linux_>::~bindable_layout_handler() = default;

void bindable_layout_handler<platform::linux_>::rebuild_children(layout& host) {
    if (native_ == nullptr) return;
    GtkBox* box = GTK_BOX(native_);
    // Drain existing children. gtk_widget_get_first_child is safe to
    // call repeatedly until it returns nullptr.
    while (GtkWidget* child = gtk_widget_get_first_child(native_)) {
        gtk_box_remove(box, child);
    }
    // M-04b: item_template instantiation deferred — see header comment.
    // The clear half of the contract still runs so future template
    // wiring starts from a known state.
    (void)host;
}

void bindable_layout_handler<platform::linux_>::map_items_source(layout& host) {
    rebuild_children(host);
}

void bindable_layout_handler<platform::linux_>::map_item_template(layout& /*host*/) {
    // Recorded but not yet driving instantiation.
}

void bindable_layout_handler<platform::linux_>::map_empty_view(layout& host) {
    if (native_ == nullptr) return;
    const auto& items = bindable_layout::get_items_source(host);
    if (!items.items.empty()) return;
    auto empty = bindable_layout::get_empty_view(host);
    view* raw = empty.get();
    if (raw == nullptr) return;
    if (GtkWidget* w = detail::linux_dispatch::dispatch(raw); w != nullptr) {
        gtk_box_append(GTK_BOX(native_), w);
    }
}

} // namespace mpapp

// ----- ADR-0013 self-registration --------------------------------------------
//
// `bindable_layout` is an attached-property facility, not a `view`
// subclass — there is no instance to `dynamic_cast` to. The dispatcher
// is therefore a defensive no-op: returns nullptr so the registry
// simply skips it. Kept to satisfy ADR-0013's self-registration
// contract.

namespace {

GtkWidget* dispatch_bindable_layout(::mpapp::view* /*v*/) {
    return nullptr;  // see comment above
}

struct registrar {
    registrar() {
        ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_bindable_layout);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
