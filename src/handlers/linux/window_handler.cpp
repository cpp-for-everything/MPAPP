// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — GTK4 basic_window handler implementation.

#include "mpapp/handlers/linux/window_handler.hpp"

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
#include "mpapp/handlers/linux/scroll_view_handler.hpp"
#include "mpapp/handlers/linux/slider_handler.hpp"
#include "mpapp/handlers/linux/stack_layout_handler.hpp"
#include "mpapp/handlers/linux/stepper_handler.hpp"
#include "mpapp/handlers/linux/switch_handler.hpp"
#include "mpapp/internal/basic_label.hpp"
#include "mpapp/internal/basic_radio_button.hpp"
#include "mpapp/internal/basic_scroll_view.hpp"
#include "mpapp/internal/basic_slider.hpp"
#include "mpapp/internal/basic_stack_layout.hpp"
#include "mpapp/internal/basic_stepper.hpp"
#include "mpapp/internal/basic_switch_.hpp"

namespace mpapp::internal {

window_handler<platform::linux_>::window_handler() = default;

window_handler<platform::linux_>::~window_handler() {
    // The GtkWidget is owned by its parent application once attached.
    // If we never attached (handler dropped before the GtkApplication
    // got it), destroy the floating reference explicitly.
    if (native_ != nullptr && gtk_app_ == nullptr) {
        gtk_window_destroy(GTK_WINDOW(static_cast<GtkWidget*>(native_)));
    }
}

void window_handler<platform::linux_>::attach_to_application(void* gtk_application) {
    gtk_app_ = gtk_application;
    if (native_ == nullptr && gtk_application != nullptr) {
        native_ = gtk_application_window_new(
            static_cast<GtkApplication*>(gtk_application));
    }
}

void window_handler<platform::linux_>::apply_title(const std::string& v) {
    if (native_ != nullptr) {
        gtk_window_set_title(GTK_WINDOW(static_cast<GtkWidget*>(native_)),
                             v.c_str());
    }
}

void window_handler<platform::linux_>::apply_content(view* v) {
    if (native_ == nullptr) {
        return;
    }
    GtkWindow* win = GTK_WINDOW(static_cast<GtkWidget*>(native_));
    if (v == nullptr) {
        gtk_window_set_child(win, nullptr);
        return;
    }
    // ADR-0013: registry first.
    if (GtkWidget* w = detail::linux_dispatch::dispatch(v); w != nullptr) {
        gtk_window_set_child(win, w);
        return;
    }
    // All concrete widgets self-register via ADR-0013; the legacy
    // dynamic_cast chain has been removed.
}

void window_handler<platform::linux_>::apply_width_or_height() {
    if (native_ == nullptr || bound_ == nullptr) {
        return;
    }
    const int w = bound_->width.get();
    const int h = bound_->height.get();
    if (w > 0 && h > 0) {
        gtk_window_set_default_size(
            GTK_WINDOW(static_cast<GtkWidget*>(native_)), w, h);
    }
}

void window_handler<platform::linux_>::apply_is_visible(bool v) {
    if (native_ == nullptr) {
        return;
    }
    GtkWindow* win = GTK_WINDOW(static_cast<GtkWidget*>(native_));
    if (v) {
        gtk_window_present(win);
    } else {
        gtk_window_close(win);
    }
}

namespace {

void on_window_close_request(GtkWindow* /*win*/, gpointer user_data) {
    auto* w = static_cast<mpapp::internal::basic_window*>(user_data);
    if (w != nullptr) {
        w->closed.emit();
    }
}

} // namespace

void window_handler<platform::linux_>::bind(basic_window& w) {
    bound_ = &w;

    if (native_ == nullptr) {
        // bind() may be called before attach_to_application. Prefer to
        // attach to the default application (set by GtkApplication's
        // activate callback) so the lifecycle and event loop are tied
        // to the framework's GtkApplication. If no default exists we
        // fall back to a free-standing GtkWindow.
        GApplication* default_app = g_application_get_default();
        if (default_app != nullptr && GTK_IS_APPLICATION(default_app)) {
            native_  = gtk_application_window_new(GTK_APPLICATION(default_app));
            gtk_app_ = default_app;
        } else {
            native_ = gtk_window_new();
        }
    }

    apply_title(w.title.get());
    w.title.changed.subscribe(title_slot_, title_cb_);

    apply_content(w.content.get());
    w.content.changed.subscribe(content_slot_, content_cb_);

    apply_width_or_height();
    w.width.changed.subscribe(width_slot_, width_cb_);
    w.height.changed.subscribe(height_slot_, height_cb_);

    apply_is_visible(w.is_visible.get());
    w.is_visible.changed.subscribe(visible_slot_, visible_cb_);

    g_signal_connect(static_cast<GtkWidget*>(native_),
                     "close-request",
                     G_CALLBACK(on_window_close_request),
                     &w);
}

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
