// SPDX-License-Identifier: Apache-2.0
// GTK4 flyout_page handler implementation.

#include "mpapp/handlers/linux/flyout_page_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp {

flyout_page_handler<platform::linux_>::flyout_page_handler() {
    native_ = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(native_), TRUE);
    gtk_widget_set_vexpand(static_cast<GtkWidget*>(native_), TRUE);

    flyout_slot_w_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    detail_slot_w_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(detail_slot_w_), TRUE);

    gtk_paned_set_start_child(GTK_PANED(static_cast<GtkWidget*>(native_)),
                              static_cast<GtkWidget*>(flyout_slot_w_));
    gtk_paned_set_end_child  (GTK_PANED(static_cast<GtkWidget*>(native_)),
                              static_cast<GtkWidget*>(detail_slot_w_));

    // Hide the flyout pane until is_presented goes true.
    gtk_widget_set_visible(static_cast<GtkWidget*>(flyout_slot_w_), FALSE);
}

flyout_page_handler<platform::linux_>::~flyout_page_handler() = default;

void flyout_page_handler<platform::linux_>::apply_flyout(page* p) {
    GtkBox* slot = GTK_BOX(static_cast<GtkWidget*>(flyout_slot_w_));
    if (current_flyout_child_ != nullptr) {
        gtk_box_remove(slot, GTK_WIDGET(current_flyout_child_));
        current_flyout_child_ = nullptr;
    }
    if (p != nullptr) {
        if (GtkWidget* w = detail::linux_dispatch::dispatch(p); w != nullptr) {
            gtk_box_append(slot, w);
            current_flyout_child_ = w;
        }
    }
}

void flyout_page_handler<platform::linux_>::apply_detail(page* p) {
    GtkBox* slot = GTK_BOX(static_cast<GtkWidget*>(detail_slot_w_));
    if (current_detail_child_ != nullptr) {
        gtk_box_remove(slot, GTK_WIDGET(current_detail_child_));
        current_detail_child_ = nullptr;
    }
    if (p != nullptr) {
        if (GtkWidget* w = detail::linux_dispatch::dispatch(p); w != nullptr) {
            gtk_box_append(slot, w);
            current_detail_child_ = w;
        }
    }
}

void flyout_page_handler<platform::linux_>::apply_is_presented(bool v) {
    if (flyout_slot_w_ == nullptr) return;
    gtk_widget_set_visible(static_cast<GtkWidget*>(flyout_slot_w_), v ? TRUE : FALSE);
}

void flyout_page_handler<platform::linux_>::map_flyout(flyout_page& fp) {
    apply_flyout(fp.flyout.get());
    fp.flyout.changed.subscribe(flyout_slot_, flyout_cb_);
}

void flyout_page_handler<platform::linux_>::map_detail(flyout_page& fp) {
    apply_detail(fp.detail.get());
    fp.detail.changed.subscribe(detail_slot_, detail_cb_);
}

void flyout_page_handler<platform::linux_>::map_is_presented(flyout_page& fp) {
    apply_is_presented(fp.is_presented.get());
    fp.is_presented.changed.subscribe(presented_slot_, presented_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_flyout_page(::mpapp::view* v) {
    if (auto* f = dynamic_cast<::mpapp::flyout_page*>(v); f && f->has_fp_handler()) {
        return GTK_WIDGET(f->fp_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_flyout_page); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
