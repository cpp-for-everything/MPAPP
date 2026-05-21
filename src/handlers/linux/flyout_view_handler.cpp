// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 flyout_view handler implementation.

#include "mpapp/handlers/linux/flyout_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp {

flyout_view_handler<platform::linux_>::flyout_view_handler() {
    GtkWidget* paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    // Allow neither pane to be fully collapsed by drag — the divider
    // stops at each side's natural minimum size. Resize/shrink defaults
    // match GtkPaned's "both sides resize, neither shrinks" behaviour
    // which is what an open drawer wants.
    gtk_paned_set_wide_handle(GTK_PANED(paned), TRUE);
    native_ = paned;
}

flyout_view_handler<platform::linux_>::~flyout_view_handler() = default;

void flyout_view_handler<platform::linux_>::apply_flyout(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    GtkPaned* paned = GTK_PANED(static_cast<GtkWidget*>(native_));

    // Detach previous start child if any. `gtk_paned_set_start_child(NULL)`
    // removes the current child; we don't need to track the old pointer
    // beyond null-resetting our cached handle.
    if (current_flyout_ != nullptr) {
        gtk_paned_set_start_child(paned, nullptr);
        current_flyout_ = nullptr;
    }

    if (!v) return;
    // ADR-0013 registry first; if no widget is registered for the child
    // type, leave the start pane empty.
    if (GtkWidget* child = detail::linux_dispatch::dispatch(v.get()); child != nullptr) {
        gtk_paned_set_start_child(paned, child);
        gtk_paned_set_resize_start_child(paned, FALSE);
        gtk_paned_set_shrink_start_child(paned, FALSE);
        current_flyout_ = child;
    }
}

void flyout_view_handler<platform::linux_>::apply_detail(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    GtkPaned* paned = GTK_PANED(static_cast<GtkWidget*>(native_));

    if (current_detail_ != nullptr) {
        gtk_paned_set_end_child(paned, nullptr);
        current_detail_ = nullptr;
    }

    if (!v) return;
    if (GtkWidget* child = detail::linux_dispatch::dispatch(v.get()); child != nullptr) {
        gtk_paned_set_end_child(paned, child);
        gtk_paned_set_resize_end_child(paned, TRUE);
        gtk_paned_set_shrink_end_child(paned, FALSE);
        current_detail_ = child;
    }
}

void flyout_view_handler<platform::linux_>::apply_is_presented(bool v) {
    if (native_ == nullptr) return;
    if (current_flyout_ == nullptr) return;
    // Toggle visibility of the start child. GtkPaned hides the divider
    // gracefully when one side is not visible.
    gtk_widget_set_visible(static_cast<GtkWidget*>(current_flyout_), v ? TRUE : FALSE);
}

void flyout_view_handler<platform::linux_>::map_flyout(flyout_view& f) {
    apply_flyout(f.flyout.get());
    f.flyout.changed.subscribe(flyout_slot_, flyout_cb_);
}

void flyout_view_handler<platform::linux_>::map_detail(flyout_view& f) {
    apply_detail(f.detail.get());
    f.detail.changed.subscribe(detail_slot_, detail_cb_);
}

void flyout_view_handler<platform::linux_>::map_is_presented(flyout_view& f) {
    apply_is_presented(f.is_presented.get());
    f.is_presented.changed.subscribe(is_presented_slot_, is_presented_cb_);
}

void flyout_view_handler<platform::linux_>::bind_flyout(flyout_view& f, view& child) {
    f.flyout.set(std::shared_ptr<view>(&child, [](view*){}));
}

void flyout_view_handler<platform::linux_>::bind_detail(flyout_view& f, view& child) {
    f.detail.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp

// ----- ADR-0013 self-registration --------------------------------------

namespace {

GtkWidget* dispatch_flyout_view(::mpapp::view* v) {
    if (auto* f = dynamic_cast<::mpapp::flyout_view*>(v); f && f->has_handler()) {
        return GTK_WIDGET(f->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_flyout_view);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
