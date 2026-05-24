// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_page handler implementation.

#include "mpapp/handlers/linux/page_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

#include "mpapp/internal/basic_page.hpp"
#include "mpapp/view.hpp"

namespace mpapp::internal {

page_handler<platform::linux_>::page_handler() {
    GtkWidget* box     = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* basic_label   = gtk_label_new("");
    GtkWidget* spinner = gtk_spinner_new();

    gtk_label_set_xalign(GTK_LABEL(basic_label), 0.0f);
    gtk_widget_set_visible(spinner, FALSE);

    // Title goes first; content is appended in apply_content(); busy
    // spinner sits at the end so it's visible above any layout slack.
    gtk_box_append(GTK_BOX(box), basic_label);
    gtk_box_append(GTK_BOX(box), spinner);

    native_       = box;
    title_label_  = basic_label;
    busy_spinner_ = spinner;
}

page_handler<platform::linux_>::~page_handler() = default;

void page_handler<platform::linux_>::apply_title(const std::string& v) {
    if (title_label_ == nullptr) return;
    gtk_label_set_text(GTK_LABEL(static_cast<GtkWidget*>(title_label_)), v.c_str());
}

void page_handler<platform::linux_>::apply_content(view* v) {
    if (native_ == nullptr) return;
    GtkBox* box = GTK_BOX(static_cast<GtkWidget*>(native_));

    if (current_child_ != nullptr) {
        gtk_box_remove(box, GTK_WIDGET(current_child_));
        current_child_ = nullptr;
    }
    if (v == nullptr) return;

    // ADR-0013: query the registry.
    GtkWidget* child = detail::linux_dispatch::dispatch(v);
    if (child == nullptr) return;

    // Insert between title and spinner.
    if (busy_spinner_ != nullptr) {
        gtk_box_insert_child_after(box, child, GTK_WIDGET(title_label_));
    } else {
        gtk_box_append(box, child);
    }
    current_child_ = child;
}

void page_handler<platform::linux_>::apply_is_busy(bool v) {
    if (busy_spinner_ == nullptr) return;
    GtkSpinner* sp = GTK_SPINNER(static_cast<GtkWidget*>(busy_spinner_));
    gtk_widget_set_visible(GTK_WIDGET(sp), v ? TRUE : FALSE);
    gtk_spinner_set_spinning(sp, v ? TRUE : FALSE);
}

void page_handler<platform::linux_>::map_title(basic_page& p) {
    apply_title(p.title.get());
    p.title.changed.subscribe(title_slot_, title_cb_);
}

void page_handler<platform::linux_>::map_content(basic_page& p) {
    apply_content(p.content.get());
    p.content.changed.subscribe(content_slot_, content_cb_);
}

void page_handler<platform::linux_>::map_is_busy(basic_page& p) {
    apply_is_busy(p.is_busy.get());
    p.is_busy.changed.subscribe(busy_slot_, busy_cb_);
}

void page_handler<platform::linux_>::bind_content(basic_page& p, view& child) {
    p.content.set(&child);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --

namespace {

GtkWidget* dispatch_page(::mpapp::view* v) {
    if (auto* p = dynamic_cast<::mpapp::internal::basic_page*>(v); p && p->has_handler()) {
        return GTK_WIDGET(p->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_page); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
