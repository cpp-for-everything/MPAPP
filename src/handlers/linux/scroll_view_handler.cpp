// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_scroll_view handler implementation.

#include "mpapp/handlers/linux/scroll_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

#include "mpapp/internal/basic_activity_indicator.hpp"
#include "mpapp/border.hpp"
#include "mpapp/internal/basic_box_view.hpp"
#include "mpapp/internal/basic_date_picker.hpp"
#include "mpapp/internal/basic_image.hpp"
#include "mpapp/internal/basic_image_button.hpp"
#include "mpapp/internal/basic_picker.hpp"
#include "mpapp/internal/basic_time_picker.hpp"
#include "mpapp/internal/basic_progress_bar.hpp"
#include "mpapp/internal/basic_search_bar.hpp"
#include "mpapp/internal/basic_button.hpp"
#include "mpapp/internal/basic_check_box.hpp"
#include "mpapp/editor.hpp"
#include "mpapp/internal/basic_entry.hpp"
#include "mpapp/handlers/linux/activity_indicator_handler.hpp"
#include "mpapp/handlers/linux/border_handler.hpp"
#include "mpapp/handlers/linux/box_view_handler.hpp"
#include "mpapp/handlers/linux/date_picker_handler.hpp"
#include "mpapp/handlers/linux/image_handler.hpp"
#include "mpapp/handlers/linux/image_button_handler.hpp"
#include "mpapp/handlers/linux/picker_handler.hpp"
#include "mpapp/handlers/linux/time_picker_handler.hpp"
#include "mpapp/handlers/linux/progress_bar_handler.hpp"
#include "mpapp/handlers/linux/search_bar_handler.hpp"
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
#include "mpapp/internal/basic_label.hpp"
#include "mpapp/internal/basic_radio_button.hpp"
#include "mpapp/internal/basic_slider.hpp"
#include "mpapp/internal/basic_stack_layout.hpp"
#include "mpapp/internal/basic_stepper.hpp"
#include "mpapp/internal/basic_switch_.hpp"

namespace mpapp::internal {

namespace {

GtkWidget* native_widget_of(view* v) {
    // ADR-0013: registry dispatch only — each widget self-registers.
    return detail::linux_dispatch::dispatch(v);
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

void scroll_view_handler<platform::linux_>::map_content(basic_scroll_view& s) {
    bound_ = &s;
    apply_content(s.content.get());
    s.content.changed.subscribe(content_slot_, content_cb_);
}

void scroll_view_handler<platform::linux_>::map_orientation(basic_scroll_view& s) {
    apply_orientation(s.orientation.get());
    s.orientation.changed.subscribe(orient_slot_, orient_cb_);
}

void scroll_view_handler<platform::linux_>::bind_content(basic_scroll_view& s, view& child) {
    s.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
namespace {

GtkWidget* dispatch_scroll_view(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::internal::basic_scroll_view*>(v); s && s->has_handler()) {
        return GTK_WIDGET(s->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_scroll_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
