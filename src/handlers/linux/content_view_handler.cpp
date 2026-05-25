// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_content_view handler implementation.

#include "mpapp/handlers/linux/content_view_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

// ADR-0013: ask the per-platform dispatch registry. Each widget's .cpp
// self-registers a dispatcher; the registry tries each in order and
// returns the first non-null. Replaces the legacy dynamic_cast chain
// that had to be edited every time a new widget landed.
GtkWidget* native_widget_of(view* v) {
    return ::mpapp::detail::linux_dispatch::dispatch(v);
}

} // namespace

content_view_handler<platform::linux_>::content_view_handler() {
    native_ = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
}

content_view_handler<platform::linux_>::~content_view_handler() = default;

void content_view_handler<platform::linux_>::apply_content(const std::shared_ptr<view>& v) {
    GtkBox* box = GTK_BOX(static_cast<GtkWidget*>(native_));
    if (current_child_ != nullptr) {
        gtk_box_remove(box, GTK_WIDGET(current_child_));
        current_child_ = nullptr;
    }
    GtkWidget* child = v ? native_widget_of(v.get()) : nullptr;
    if (child != nullptr) {
        gtk_box_append(box, child);
        current_child_ = child;
    }
}

void content_view_handler<platform::linux_>::map_content(basic_content_view& c) {
    apply_content(c.content.get());
    c.content.changed.subscribe(content_slot_, content_cb_);
}

void content_view_handler<platform::linux_>::bind_content(basic_content_view& c, view& child) {
    c.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

void content_view_handler<platform::linux_>::map_gestures(basic_content_view& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
#include "mpapp/internal/basic_content_view.hpp"

namespace {

GtkWidget* dispatch_content_view(::mpapp::view* v) {
    if (auto* cv = dynamic_cast<::mpapp::internal::basic_content_view*>(v); cv && cv->has_handler()) {
        return GTK_WIDGET(cv->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_content_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
