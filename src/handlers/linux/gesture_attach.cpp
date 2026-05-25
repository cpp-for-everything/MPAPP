// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 implementation of RFC-0003 gesture recognizers.

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include "mpapp/gestures/pan_gesture_recognizer.hpp"
#include "mpapp/gestures/pinch_gesture_recognizer.hpp"
#include "mpapp/gestures/pointer_gesture_recognizer.hpp"
#include "mpapp/gestures/swipe_gesture_recognizer.hpp"
#include "mpapp/gestures/tap_gesture_recognizer.hpp"
#include "mpapp/view.hpp"

namespace mpapp::internal::linux_gestures {

namespace {

// ---------- Tap ------------------------------------------------------------
//
// `gtk_gesture_click_new()` fires `pressed` with (n_press, x, y); `n_press`
// is the multi-click count (matches MAUI's NumberOfTapsRequired contract).
// We fire `tap.tapped` only when `n_press` equals the configured count, so
// the recognizer reports double-tap exactly once on the second press —
// matching MAUI's semantics.

void on_tap_pressed(GtkGestureClick* /*g*/,
                    gint n_press,
                    gdouble x, gdouble y,
                    gpointer user_data) {
    auto* tap = static_cast<tap_gesture_recognizer*>(user_data);
    if (tap == nullptr) return;
    if (n_press != tap->number_of_taps_required.get()) return;
    tap->tapped.emit(tapped_event_args{
        x, y,
        button_mask::primary,   // GtkGestureClick default is primary;
                                // multi-button discrimination via the
                                // GdkButton event lands with PointerGesture.
    });
}

void install_tap(GtkWidget* widget, tap_gesture_recognizer& tap) {
    GtkGesture* g = gtk_gesture_click_new();
    g_signal_connect(g, "pressed", G_CALLBACK(on_tap_pressed), &tap);
    gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(g));
}

// ---------- Pan ------------------------------------------------------------
//
// `gtk_gesture_drag_new()` reports drag-relative deltas; we emit the
// three lifecycle ticks (started / running / completed) so MAUI consumers
// observing `PanUpdatedEventArgs.StatusType` see the same transitions
// they would on other platforms.

struct pan_ctx {
    pan_gesture_recognizer* pan;
    int                     gesture_id;
};

void on_pan_begin(GtkGestureDrag* /*g*/, gdouble /*x*/, gdouble /*y*/, gpointer ud) {
    auto* c = static_cast<pan_ctx*>(ud);
    c->gesture_id++;
    c->pan->pan_updated.emit(pan_updated_event_args{
        gesture_status::started, c->gesture_id, 0.0, 0.0,
    });
}

void on_pan_update(GtkGestureDrag* /*g*/, gdouble dx, gdouble dy, gpointer ud) {
    auto* c = static_cast<pan_ctx*>(ud);
    c->pan->pan_updated.emit(pan_updated_event_args{
        gesture_status::running, c->gesture_id, dx, dy,
    });
}

void on_pan_end(GtkGestureDrag* /*g*/, gdouble dx, gdouble dy, gpointer ud) {
    auto* c = static_cast<pan_ctx*>(ud);
    c->pan->pan_updated.emit(pan_updated_event_args{
        gesture_status::completed, c->gesture_id, dx, dy,
    });
}

void install_pan(GtkWidget* widget, pan_gesture_recognizer& pan) {
    auto* ctx = new pan_ctx{&pan, 0};
    GtkGesture* g = gtk_gesture_drag_new();
    g_signal_connect_data(g, "drag-begin",  G_CALLBACK(on_pan_begin),  ctx,
                          +[](gpointer p, GClosure*) { delete static_cast<pan_ctx*>(p); },
                          static_cast<GConnectFlags>(0));
    g_signal_connect(g, "drag-update", G_CALLBACK(on_pan_update), ctx);
    g_signal_connect(g, "drag-end",    G_CALLBACK(on_pan_end),    ctx);
    gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(g));
}

// ---------- Pinch ----------------------------------------------------------
//
// `gtk_gesture_zoom_new()` reports a multiplicative scale relative to the
// gesture's start; MAUI's contract is the same.

struct pinch_ctx {
    pinch_gesture_recognizer* pinch;
    gesture_status            phase;
};

void on_pinch_scale_changed(GtkGestureZoom* g, gdouble scale, gpointer ud) {
    auto* c = static_cast<pinch_ctx*>(ud);
    // Approximate the centroid of the two contact points in normalised
    // view-local coords. GTK4 doesn't surface a per-tick centroid for
    // GtkGestureZoom, so we use the gesture's bounding-box centre
    // queried via gtk_gesture_get_bounding_box_center.
    gdouble cx = 0.5, cy = 0.5;
    gdouble bx = 0.0, by = 0.0;
    if (gtk_gesture_get_bounding_box_center(GTK_GESTURE(g), &bx, &by) == TRUE) {
        GtkWidget* w = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(g));
        int width  = gtk_widget_get_width(w);
        int height = gtk_widget_get_height(w);
        if (width  > 0) cx = bx / static_cast<gdouble>(width);
        if (height > 0) cy = by / static_cast<gdouble>(height);
    }
    c->pinch->pinch_updated.emit(pinch_updated_event_args{
        c->phase == gesture_status::started
            ? gesture_status::running
            : c->phase,
        scale, cx, cy,
    });
    c->phase = gesture_status::running;
}

void on_pinch_begin(GtkGesture* /*g*/, GdkEventSequence* /*seq*/, gpointer ud) {
    auto* c = static_cast<pinch_ctx*>(ud);
    c->phase = gesture_status::started;
    c->pinch->pinch_updated.emit(pinch_updated_event_args{
        gesture_status::started, 1.0, 0.5, 0.5,
    });
}

void on_pinch_end(GtkGesture* /*g*/, GdkEventSequence* /*seq*/, gpointer ud) {
    auto* c = static_cast<pinch_ctx*>(ud);
    c->phase = gesture_status::completed;
    c->pinch->pinch_updated.emit(pinch_updated_event_args{
        gesture_status::completed, 1.0, 0.5, 0.5,
    });
}

void install_pinch(GtkWidget* widget, pinch_gesture_recognizer& pinch) {
    auto* ctx = new pinch_ctx{&pinch, gesture_status::started};
    GtkGesture* g = gtk_gesture_zoom_new();
    g_signal_connect_data(g, "scale-changed", G_CALLBACK(on_pinch_scale_changed), ctx,
                          +[](gpointer p, GClosure*) { delete static_cast<pinch_ctx*>(p); },
                          static_cast<GConnectFlags>(0));
    g_signal_connect(g, "begin", G_CALLBACK(on_pinch_begin), ctx);
    g_signal_connect(g, "end",   G_CALLBACK(on_pinch_end),   ctx);
    gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(g));
}

// ---------- Swipe ----------------------------------------------------------
//
// `gtk_gesture_swipe_new()` reports the final fling velocity in pixels-
// per-second; we derive a single dominant direction from the velocity
// vector and gate on the recognizer's `direction` bitmask + threshold.

void on_swipe(GtkGestureSwipe* /*g*/, gdouble vx, gdouble vy, gpointer ud) {
    auto* sw = static_cast<swipe_gesture_recognizer*>(ud);
    if (sw == nullptr) return;

    // Pick dominant axis + sign for the direction bit; fall through if
    // the velocity is too low.
    const double abs_vx = vx < 0 ? -vx : vx;
    const double abs_vy = vy < 0 ? -vy : vy;
    const double magnitude = abs_vx > abs_vy ? abs_vx : abs_vy;
    if (magnitude < static_cast<double>(sw->threshold.get())) return;

    swipe_direction dir = swipe_direction::none;
    if (abs_vx > abs_vy) {
        dir = vx > 0 ? swipe_direction::right : swipe_direction::left;
    } else {
        dir = vy > 0 ? swipe_direction::down  : swipe_direction::up;
    }
    if (!any(sw->direction.get(), dir)) return;
    sw->swiped.emit(swiped_event_args{dir});
}

void install_swipe(GtkWidget* widget, swipe_gesture_recognizer& sw) {
    GtkGesture* g = gtk_gesture_swipe_new();
    g_signal_connect(g, "swipe", G_CALLBACK(on_swipe), &sw);
    gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(g));
}

// ---------- Pointer --------------------------------------------------------
//
// Two GTK controllers: `GtkEventControllerMotion` for enter/exit/move,
// and `GtkGestureClick` for press/release. We attach both and bind their
// signals into the same `pointer_gesture_recognizer`.

void on_pointer_enter(GtkEventControllerMotion* /*c*/, gdouble x, gdouble y, gpointer ud) {
    auto* p = static_cast<pointer_gesture_recognizer*>(ud);
    p->pointer_entered.emit(pointer_event_args{x, y, button_mask::none});
}
void on_pointer_motion(GtkEventControllerMotion* /*c*/, gdouble x, gdouble y, gpointer ud) {
    auto* p = static_cast<pointer_gesture_recognizer*>(ud);
    p->pointer_moved.emit(pointer_event_args{x, y, button_mask::none});
}
void on_pointer_leave(GtkEventControllerMotion* /*c*/, gpointer ud) {
    auto* p = static_cast<pointer_gesture_recognizer*>(ud);
    // GTK4's "leave" signal doesn't carry a coordinate — report 0,0.
    p->pointer_exited.emit(pointer_event_args{0.0, 0.0, button_mask::none});
}

void on_pointer_pressed(GtkGestureClick* /*g*/,
                        gint /*n_press*/, gdouble x, gdouble y,
                        gpointer ud) {
    auto* p = static_cast<pointer_gesture_recognizer*>(ud);
    p->pointer_pressed.emit(pointer_event_args{x, y, button_mask::primary});
}
void on_pointer_released(GtkGestureClick* /*g*/,
                         gint /*n_press*/, gdouble x, gdouble y,
                         gpointer ud) {
    auto* p = static_cast<pointer_gesture_recognizer*>(ud);
    p->pointer_released.emit(pointer_event_args{x, y, button_mask::primary});
}

void install_pointer(GtkWidget* widget, pointer_gesture_recognizer& ptr) {
    GtkEventController* mc = gtk_event_controller_motion_new();
    g_signal_connect(mc, "enter",  G_CALLBACK(on_pointer_enter),  &ptr);
    g_signal_connect(mc, "motion", G_CALLBACK(on_pointer_motion), &ptr);
    g_signal_connect(mc, "leave",  G_CALLBACK(on_pointer_leave),  &ptr);
    gtk_widget_add_controller(widget, mc);

    GtkGesture* cg = gtk_gesture_click_new();
    g_signal_connect(cg, "pressed",  G_CALLBACK(on_pointer_pressed),  &ptr);
    g_signal_connect(cg, "released", G_CALLBACK(on_pointer_released), &ptr);
    gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(cg));
}

} // namespace

void attach(GtkWidget* widget, view& v) {
    if (widget == nullptr) return;
    for (const auto& r : v.gesture_recognizers) {
        switch (r->kind()) {
            case gesture_kind::tap:
                install_tap(widget, static_cast<tap_gesture_recognizer&>(*r));
                break;
            case gesture_kind::pan:
                install_pan(widget, static_cast<pan_gesture_recognizer&>(*r));
                break;
            case gesture_kind::pinch:
                install_pinch(widget, static_cast<pinch_gesture_recognizer&>(*r));
                break;
            case gesture_kind::swipe:
                install_swipe(widget, static_cast<swipe_gesture_recognizer&>(*r));
                break;
            case gesture_kind::pointer:
                install_pointer(widget, static_cast<pointer_gesture_recognizer&>(*r));
                break;
        }
    }
}

} // namespace mpapp::internal::linux_gestures

#endif // __linux__ && !__ANDROID__
