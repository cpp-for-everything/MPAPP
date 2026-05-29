// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_carousel_view handler implementation.

#include "mpapp/handlers/linux/carousel_view_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <cmath>
#include <string>

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

void clear_stack(GtkStack* stack) {
    GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(stack));
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_stack_remove(stack, child);
        child = next;
    }
}

// Horizontal fling on the stack → advance/retreat one page through
// scroll_to (which applies loop/clamp + emits position_changed). The
// position.changed subscription then drives the visible-child switch.
void on_swipe(GtkGestureSwipe* /*g*/, gdouble vel_x, gdouble vel_y,
              gpointer user_data) {
    auto* c = static_cast<basic_carousel_view*>(user_data);
    if (c == nullptr) return;
    if (!c->is_swipe_enabled.get()) return;
    // Ignore mostly-vertical flings.
    if (std::abs(vel_x) < std::abs(vel_y)) return;
    const int delta = (vel_x < 0.0) ? +1 : -1;  // fling left → next page
    c->scroll_to(c->position.get() + delta);
}

} // namespace

carousel_view_handler<platform::linux_>::carousel_view_handler() {
    native_ = gtk_stack_new();
    GtkStack* stack = GTK_STACK(static_cast<GtkWidget*>(native_));
    gtk_stack_set_transition_type(stack, GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_widget_set_vexpand(static_cast<GtkWidget*>(native_), TRUE);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(native_), TRUE);

    GtkGesture* g = gtk_gesture_swipe_new();
    swipe_ctrl_ = g;
    gtk_widget_add_controller(static_cast<GtkWidget*>(native_),
                              GTK_EVENT_CONTROLLER(g));
}

carousel_view_handler<platform::linux_>::~carousel_view_handler() = default;

void carousel_view_handler<platform::linux_>::rebuild_pages(
    const std::vector<std::string>& v) {
    if (native_ == nullptr) return;
    GtkStack* stack = GTK_STACK(static_cast<GtkWidget*>(native_));
    clear_stack(stack);
    int i = 0;
    for (const auto& s : v) {
        GtkWidget* lbl = gtk_label_new(s.c_str());
        gtk_widget_set_halign(lbl, GTK_ALIGN_CENTER);
        gtk_widget_set_valign(lbl, GTK_ALIGN_CENTER);
        const std::string name = std::to_string(i++);
        gtk_stack_add_named(stack, lbl, name.c_str());
    }
}

void carousel_view_handler<platform::linux_>::rebuild_after_items() {
    rebuild_pages(bound_ != nullptr ? bound_->items_source.get()
                                    : std::vector<std::string>{});
    if (bound_ != nullptr) apply_position(bound_->position.get());
}

void carousel_view_handler<platform::linux_>::apply_position(int idx) {
    if (native_ == nullptr) return;
    GtkStack* stack = GTK_STACK(static_cast<GtkWidget*>(native_));
    const std::string name = std::to_string(idx);
    GtkWidget* page = gtk_stack_get_child_by_name(stack, name.c_str());
    if (page != nullptr) {
        gtk_stack_set_visible_child(stack, page);
    }
}

void carousel_view_handler<platform::linux_>::apply_swipe_enabled(bool on) {
    if (swipe_ctrl_ == nullptr) return;
    gtk_event_controller_set_propagation_phase(
        GTK_EVENT_CONTROLLER(static_cast<GtkGesture*>(swipe_ctrl_)),
        on ? GTK_PHASE_BUBBLE : GTK_PHASE_NONE);
}

void carousel_view_handler<platform::linux_>::map_items_source(basic_carousel_view& c) {
    bound_ = &c;
    g_signal_connect(static_cast<GtkGesture*>(swipe_ctrl_), "swipe",
                     G_CALLBACK(on_swipe), &c);
    rebuild_after_items();
    c.items_source.changed.subscribe(items_slot_, items_cb_);
}

void carousel_view_handler<platform::linux_>::map_position(basic_carousel_view& c) {
    apply_position(c.position.get());
    c.position.changed.subscribe(pos_slot_, pos_cb_);
}

void carousel_view_handler<platform::linux_>::map_loop(basic_carousel_view& /*c*/) {
    // Loop/clamp logic lives in basic_carousel_view::scroll_to; nothing
    // to wire on the native side. (Subscribing is unnecessary — the
    // surface consults loop.get() at scroll time.)
}

void carousel_view_handler<platform::linux_>::map_is_swipe_enabled(basic_carousel_view& c) {
    apply_swipe_enabled(c.is_swipe_enabled.get());
    c.is_swipe_enabled.changed.subscribe(swipe_slot_, swipe_cb_);
}

void carousel_view_handler<platform::linux_>::map_peek_count(basic_carousel_view& /*c*/) {
    // GtkStack shows exactly one page — peek is a no-op on GTK v1.
    // Real peek-area insets need Adw.Carousel (libadwaita) — follow-up.
}

void carousel_view_handler<platform::linux_>::map_gestures(basic_carousel_view& c) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), c);
}

} // namespace mpapp::internal

// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_carousel_view(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::internal::basic_carousel_view*>(v);
        c && c->has_handler()) {
        return GTK_WIDGET(c->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_carousel_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
