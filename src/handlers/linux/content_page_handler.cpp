// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_content_page handler implementation.

#include "mpapp/handlers/linux/content_page_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

#include "mpapp/internal/basic_content_page.hpp"
#include "mpapp/view.hpp"

namespace mpapp::internal {

content_page_handler<platform::linux_>::content_page_handler() {
    GtkWidget* box   = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* basic_label = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(basic_label), 0.0f);
    gtk_box_append(GTK_BOX(box), basic_label);

    native_      = box;
    title_label_ = basic_label;
}

content_page_handler<platform::linux_>::~content_page_handler() = default;

void content_page_handler<platform::linux_>::apply_title(const std::string& v) {
    if (title_label_ == nullptr) return;
    gtk_label_set_text(GTK_LABEL(static_cast<GtkWidget*>(title_label_)), v.c_str());
}

void content_page_handler<platform::linux_>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    GtkBox* box = GTK_BOX(static_cast<GtkWidget*>(native_));
    if (current_child_ != nullptr) {
        gtk_box_remove(box, GTK_WIDGET(current_child_));
        current_child_ = nullptr;
    }
    GtkWidget* child = v ? detail::linux_dispatch::dispatch(v.get()) : nullptr;
    if (child != nullptr) {
        gtk_box_append(box, child);
        current_child_ = child;
    }
}

void content_page_handler<platform::linux_>::apply_padding(const thickness& t) {
    if (native_ == nullptr) return;
    GtkWidget* w = static_cast<GtkWidget*>(native_);
    gtk_widget_set_margin_start (w, static_cast<int>(t.left   + 0.5));
    gtk_widget_set_margin_top   (w, static_cast<int>(t.top    + 0.5));
    gtk_widget_set_margin_end   (w, static_cast<int>(t.right  + 0.5));
    gtk_widget_set_margin_bottom(w, static_cast<int>(t.bottom + 0.5));
}

void content_page_handler<platform::linux_>::map_title(basic_content_page& p) {
    apply_title(p.title.get());
    p.title.changed.subscribe(title_slot_, title_cb_);
}

void content_page_handler<platform::linux_>::map_content(basic_content_page& p) {
    apply_content(p.content.get());
    p.content.changed.subscribe(content_slot_, content_cb_);
}

void content_page_handler<platform::linux_>::map_padding(basic_content_page& p) {
    apply_padding(p.padding.get());
    p.padding.changed.subscribe(padding_slot_, padding_cb_);
}

void content_page_handler<platform::linux_>::bind_content(basic_content_page& p, view& child) {
    p.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

void content_page_handler<platform::linux_>::map_gestures(basic_content_page& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --

namespace {

GtkWidget* dispatch_content_page(::mpapp::view* v) {
    if (auto* cp = dynamic_cast<::mpapp::internal::basic_content_page*>(v); cp && cp->has_handler()) {
        return GTK_WIDGET(cp->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_content_page); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
