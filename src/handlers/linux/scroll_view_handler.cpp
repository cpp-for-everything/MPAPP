// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 scroll_view handler implementation.

#include "mpapp/handlers/linux/scroll_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/border.hpp"
#include "mpapp/box_view.hpp"
#include "mpapp/button.hpp"
#include "mpapp/check_box.hpp"
#include "mpapp/editor.hpp"
#include "mpapp/entry.hpp"
#include "mpapp/handlers/linux/border_handler.hpp"
#include "mpapp/handlers/linux/box_view_handler.hpp"
#include "mpapp/handlers/linux/button_handler.hpp"
#include "mpapp/handlers/linux/check_box_handler.hpp"
#include "mpapp/handlers/linux/editor_handler.hpp"
#include "mpapp/handlers/linux/entry_handler.hpp"
#include "mpapp/handlers/linux/label_handler.hpp"
#include "mpapp/handlers/linux/radio_button_handler.hpp"
#include "mpapp/handlers/linux/slider_handler.hpp"
#include "mpapp/handlers/linux/stack_layout_handler.hpp"
#include "mpapp/handlers/linux/stepper_handler.hpp"
#include "mpapp/handlers/linux/switch_handler.hpp"
#include "mpapp/label.hpp"
#include "mpapp/radio_button.hpp"
#include "mpapp/slider.hpp"
#include "mpapp/stack_layout.hpp"
#include "mpapp/stepper.hpp"
#include "mpapp/switch_.hpp"

namespace mpapp {

namespace {

GtkWidget* native_widget_of(view* v) {
    if (auto* sl = dynamic_cast<stack_layout*>(v); sl && sl->has_handler()) return GTK_WIDGET(sl->handler().native());
    if (auto* b  = dynamic_cast<button*>(v);       b  && b->has_handler())  return GTK_WIDGET(b->handler().native());
    if (auto* l  = dynamic_cast<label*>(v);        l  && l->has_handler())  return GTK_WIDGET(l->handler().native());
    if (auto* e  = dynamic_cast<entry*>(v);        e  && e->has_handler())  return GTK_WIDGET(e->handler().native());
    if (auto* sw = dynamic_cast<switch_*>(v);      sw && sw->has_handler()) return GTK_WIDGET(sw->handler().native());
    if (auto* cb = dynamic_cast<check_box*>(v);    cb && cb->has_handler()) return GTK_WIDGET(cb->handler().native());
    if (auto* rb = dynamic_cast<radio_button*>(v); rb && rb->has_handler()) return GTK_WIDGET(rb->handler().native());
    if (auto* s2 = dynamic_cast<slider*>(v);       s2 && s2->has_handler()) return GTK_WIDGET(s2->handler().native());
    if (auto* st = dynamic_cast<stepper*>(v);      st && st->has_handler()) return GTK_WIDGET(st->handler().native());
    if (auto* ed = dynamic_cast<editor*>(v);       ed && ed->has_handler()) return GTK_WIDGET(ed->handler().native());
    if (auto* bx = dynamic_cast<box_view*>(v);     bx && bx->has_handler()) return GTK_WIDGET(bx->handler().native());
    if (auto* br = dynamic_cast<border*>(v);       br && br->has_handler()) return GTK_WIDGET(br->handler().native());
    return nullptr;
}

} // namespace

scroll_view_handler<platform::linux_>::scroll_view_handler() {
    native_ = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(static_cast<GtkWidget*>(native_), TRUE);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(native_), TRUE);
}

scroll_view_handler<platform::linux_>::~scroll_view_handler() = default;

void scroll_view_handler<platform::linux_>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    GtkScrolledWindow* sw = GTK_SCROLLED_WINDOW(static_cast<GtkWidget*>(native_));
    if (!v) {
        gtk_scrolled_window_set_child(sw, nullptr);
        return;
    }
    GtkWidget* child = native_widget_of(v.get());
    if (child != nullptr) gtk_scrolled_window_set_child(sw, child);
}

void scroll_view_handler<platform::linux_>::apply_orientation(scroll_orientation o) {
    if (native_ == nullptr) return;
    GtkScrolledWindow* sw = GTK_SCROLLED_WINDOW(static_cast<GtkWidget*>(native_));
    switch (o) {
        case scroll_orientation::vertical:
            gtk_scrolled_window_set_policy(sw, GTK_POLICY_NEVER,    GTK_POLICY_AUTOMATIC); break;
        case scroll_orientation::horizontal:
            gtk_scrolled_window_set_policy(sw, GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);    break;
        case scroll_orientation::both:
            gtk_scrolled_window_set_policy(sw, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); break;
        case scroll_orientation::neither:
            gtk_scrolled_window_set_policy(sw, GTK_POLICY_NEVER,     GTK_POLICY_NEVER);    break;
    }
}

void scroll_view_handler<platform::linux_>::map_content(scroll_view& s) {
    bound_ = &s;
    apply_content(s.content.get());
    s.content.changed.subscribe(content_slot_, content_cb_);
}

void scroll_view_handler<platform::linux_>::map_orientation(scroll_view& s) {
    apply_orientation(s.orientation.get());
    s.orientation.changed.subscribe(orient_slot_, orient_cb_);
}

void scroll_view_handler<platform::linux_>::bind_content(scroll_view& s, view& child) {
    s.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
