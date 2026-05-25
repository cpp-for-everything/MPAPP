// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_swipe_item_view handler implementation.

#include "mpapp/handlers/linux/swipe_item_view_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

swipe_item_view_handler<platform::linux_>::swipe_item_view_handler() {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    native_ = box;
}

swipe_item_view_handler<platform::linux_>::~swipe_item_view_handler() = default;

void swipe_item_view_handler<platform::linux_>::apply_content(view* v) {
    if (native_ == nullptr) return;
    GtkBox* box = GTK_BOX(static_cast<GtkWidget*>(native_));

    if (current_child_ != nullptr) {
        gtk_box_remove(box, GTK_WIDGET(current_child_));
        current_child_ = nullptr;
    }
    if (v == nullptr) return;

    if (GtkWidget* child = detail::linux_dispatch::dispatch(v); child != nullptr) {
        gtk_box_append(box, child);
        current_child_ = child;
    }
}

void swipe_item_view_handler<platform::linux_>::map_content(basic_swipe_item_view& iv) {
    apply_content(iv.content.get());
    iv.content.changed.subscribe(content_slot_, content_cb_);
}

void swipe_item_view_handler<platform::linux_>::map_gestures(basic_swipe_item_view& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ----- ADR-0013 self-registration --------------------------------------

namespace {

GtkWidget* dispatch_swipe_item_view(::mpapp::view* v) {
    if (auto* iv = dynamic_cast<::mpapp::internal::basic_swipe_item_view*>(v); iv && iv->has_handler()) {
        return GTK_WIDGET(iv->handler().native());
    }
    return nullptr;
}

struct swipe_item_view_registrar {
    swipe_item_view_registrar() {
        ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_swipe_item_view);
    }
};

[[maybe_unused]] swipe_item_view_registrar _swipe_item_view_reg;

} // namespace

#endif // __linux__ && !__ANDROID__
