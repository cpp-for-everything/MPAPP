// SPDX-License-Identifier: Apache-2.0
// GTK4 content_view handler implementation.

#include "mpapp/handlers/linux/content_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/box_view.hpp"
#include "mpapp/button.hpp"
#include "mpapp/check_box.hpp"
#include "mpapp/editor.hpp"
#include "mpapp/entry.hpp"
#include "mpapp/handlers/linux/box_view_handler.hpp"
#include "mpapp/handlers/linux/button_handler.hpp"
#include "mpapp/handlers/linux/check_box_handler.hpp"
#include "mpapp/handlers/linux/editor_handler.hpp"
#include "mpapp/handlers/linux/entry_handler.hpp"
#include "mpapp/handlers/linux/label_handler.hpp"
#include "mpapp/handlers/linux/radio_button_handler.hpp"
#include "mpapp/handlers/linux/slider_handler.hpp"
#include "mpapp/handlers/linux/stack_layout_handler.hpp"
#include "mpapp/handlers/linux/switch_handler.hpp"
#include "mpapp/label.hpp"
#include "mpapp/radio_button.hpp"
#include "mpapp/slider.hpp"
#include "mpapp/stack_layout.hpp"
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
    if (auto* ed = dynamic_cast<editor*>(v);       ed && ed->has_handler()) return GTK_WIDGET(ed->handler().native());
    if (auto* bx = dynamic_cast<box_view*>(v);     bx && bx->has_handler()) return GTK_WIDGET(bx->handler().native());
    return nullptr;
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

void content_view_handler<platform::linux_>::map_content(content_view& c) {
    apply_content(c.content.get());
    c.content.changed.subscribe(content_slot_, content_cb_);
}

void content_view_handler<platform::linux_>::bind_content(content_view& c, view& child) {
    c.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
