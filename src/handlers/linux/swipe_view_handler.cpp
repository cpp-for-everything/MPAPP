// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 swipe_view handler implementation.

#include "mpapp/handlers/linux/swipe_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <vector>

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp {

swipe_view_handler<platform::linux_>::swipe_view_handler() {
    // Vertical GtkBox host — content fills the available space. The
    // action items are tracked via the registry but are not rendered
    // inline; gesture-reveal lands in a follow-up batch.
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    native_ = box;
}

swipe_view_handler<platform::linux_>::~swipe_view_handler() = default;

void swipe_view_handler<platform::linux_>::apply_content(view* v) {
    if (native_ == nullptr) return;
    GtkBox* box = GTK_BOX(static_cast<GtkWidget*>(native_));

    if (current_child_ != nullptr) {
        gtk_box_remove(box, GTK_WIDGET(current_child_));
        current_child_ = nullptr;
    }
    if (v == nullptr) return;

    // ADR-0013: query the registry.
    if (GtkWidget* child = detail::linux_dispatch::dispatch(v); child != nullptr) {
        gtk_box_append(box, child);
        current_child_ = child;
    }
}

void swipe_view_handler<platform::linux_>::apply_left_items(const std::vector<view*>& items) {
    // GTK4 gesture-reveal is deferred — the items are intentionally not
    // attached to the host yet. Touching the registry on each entry is
    // still useful so any child handler that needs its native handle
    // materialised (e.g. for an out-of-band parent test) has it.
    for (view* v : items) {
        if (v != nullptr) (void)detail::linux_dispatch::dispatch(v);
    }
}

void swipe_view_handler<platform::linux_>::apply_right_items(const std::vector<view*>& items) {
    for (view* v : items) {
        if (v != nullptr) (void)detail::linux_dispatch::dispatch(v);
    }
}

void swipe_view_handler<platform::linux_>::map_content(swipe_view& sv) {
    apply_content(sv.content.get());
    sv.content.changed.subscribe(content_slot_, content_cb_);
}

void swipe_view_handler<platform::linux_>::map_left_items(swipe_view& sv) {
    apply_left_items(sv.left_items.get());
    sv.left_items.changed.subscribe(left_slot_, left_cb_);
}

void swipe_view_handler<platform::linux_>::map_right_items(swipe_view& sv) {
    apply_right_items(sv.right_items.get());
    sv.right_items.changed.subscribe(right_slot_, right_cb_);
}

} // namespace mpapp

// ----- ADR-0013 self-registration --------------------------------------

namespace {

GtkWidget* dispatch_swipe_view(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::swipe_view*>(v); s && s->has_handler()) {
        return GTK_WIDGET(s->handler().native());
    }
    return nullptr;
}

struct swipe_view_registrar {
    swipe_view_registrar() {
        ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_swipe_view);
    }
};

[[maybe_unused]] swipe_view_registrar _swipe_view_reg;

} // namespace

#endif // __linux__ && !__ANDROID__
