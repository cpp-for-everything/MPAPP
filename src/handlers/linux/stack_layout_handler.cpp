// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — GTK4 basic_stack_layout handler implementation.

#include "mpapp/handlers/linux/stack_layout_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

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
#include "mpapp/handlers/linux/stepper_handler.hpp"
#include "mpapp/handlers/linux/switch_handler.hpp"
#include "mpapp/internal/basic_label.hpp"
#include "mpapp/internal/basic_radio_button.hpp"
#include "mpapp/internal/basic_slider.hpp"
#include "mpapp/internal/basic_stepper.hpp"
#include "mpapp/internal/basic_switch_.hpp"

namespace mpapp::internal {

namespace {

GtkAlign to_native(h_align a) noexcept {
    switch (a) {
        case h_align::start:   return GTK_ALIGN_START;
        case h_align::center:  return GTK_ALIGN_CENTER;
        case h_align::end:     return GTK_ALIGN_END;
        case h_align::stretch: return GTK_ALIGN_FILL;
    }
    return GTK_ALIGN_FILL;
}

GtkAlign to_native(v_align a) noexcept {
    switch (a) {
        case v_align::start:   return GTK_ALIGN_START;
        case v_align::center:  return GTK_ALIGN_CENTER;
        case v_align::end:     return GTK_ALIGN_END;
        case v_align::stretch: return GTK_ALIGN_FILL;
    }
    return GTK_ALIGN_FILL;
}

} // namespace

stack_layout_handler<platform::linux_>::stack_layout_handler() {
    native_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
}

stack_layout_handler<platform::linux_>::~stack_layout_handler() = default;

void stack_layout_handler<platform::linux_>::apply_orientation(orientation o) {
    if (native_ == nullptr) return;
    gtk_orientable_set_orientation(
        GTK_ORIENTABLE(static_cast<GtkWidget*>(native_)),
        o == orientation::horizontal ? GTK_ORIENTATION_HORIZONTAL
                                     : GTK_ORIENTATION_VERTICAL);
}

void stack_layout_handler<platform::linux_>::apply_spacing(double s) {
    if (native_ == nullptr) return;
    gtk_box_set_spacing(GTK_BOX(static_cast<GtkWidget*>(native_)),
                        static_cast<int>(s));
}

void stack_layout_handler<platform::linux_>::apply_padding(thickness t) {
    if (native_ == nullptr) return;
    GtkWidget* w = static_cast<GtkWidget*>(native_);
    gtk_widget_set_margin_start(w,  static_cast<int>(t.left));
    gtk_widget_set_margin_top(w,    static_cast<int>(t.top));
    gtk_widget_set_margin_end(w,    static_cast<int>(t.right));
    gtk_widget_set_margin_bottom(w, static_cast<int>(t.bottom));
}

void stack_layout_handler<platform::linux_>::apply_horizontal_alignment(h_align a) {
    if (native_ == nullptr) return;
    gtk_widget_set_halign(static_cast<GtkWidget*>(native_), to_native(a));
}

void stack_layout_handler<platform::linux_>::apply_vertical_alignment(v_align a) {
    if (native_ == nullptr) return;
    gtk_widget_set_valign(static_cast<GtkWidget*>(native_), to_native(a));
}

void stack_layout_handler<platform::linux_>::bind(basic_stack_layout& s) {
    bound_ = &s;

    apply_orientation(s.stack_orientation.get());
    s.stack_orientation.changed.subscribe(orient_slot_, orient_cb_);

    apply_spacing(s.spacing.get());
    s.spacing.changed.subscribe(spacing_slot_, spacing_cb_);

    apply_padding(s.padding.get());
    s.padding.changed.subscribe(padding_slot_, padding_cb_);

    apply_horizontal_alignment(s.horizontal_alignment.get());
    s.horizontal_alignment.changed.subscribe(h_align_slot_, h_align_cb_);

    apply_vertical_alignment(s.vertical_alignment.get());
    s.vertical_alignment.changed.subscribe(v_align_slot_, v_align_cb_);

    for (std::size_t i = 0; i < s.child_count(); ++i) {
        if (view* child = s.child_at(i); child != nullptr) {
            add_child(*child);
        }
    }
}

void stack_layout_handler<platform::linux_>::add_child(view& child) {
    if (native_ == nullptr) return;
    GtkBox* box = GTK_BOX(static_cast<GtkWidget*>(native_));

    // ADR-0013: try registry first.
    if (GtkWidget* w = detail::linux_dispatch::dispatch(&child); w != nullptr) {
        gtk_box_append(box, w);
        return;
    }

    // Unknown subtype — silently drop. All concrete widgets now self-register
    // via ADR-0013; the legacy dynamic_cast chain has been removed.
}

void stack_layout_handler<platform::linux_>::map_gestures(basic_stack_layout& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
namespace {

GtkWidget* dispatch_stack_layout(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::internal::basic_stack_layout*>(v); s && s->has_handler()) {
        return GTK_WIDGET(s->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_stack_layout); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
