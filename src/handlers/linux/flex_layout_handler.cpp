// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_flex_layout handler implementation.
//
// GTK4 ships no native flexbox container. This v1 implementation maps the
// flex container/child properties onto a single GtkBox:
//
//   * flex_direction  -> GtkOrientation (+ child ordering is left to GTK
//                        append order; *_reverse is approximated by laying
//                        children out and relying on box order).
//   * flex_wrap       -> single-line only; GtkBox cannot wrap. Documented
//                        as a follow-up (a custom GtkLayoutManager would be
//                        required for true multi-line wrap).
//   * justify_content -> main-axis halign/valign of the box itself.
//   * align_items     -> cross-axis halign/valign applied to children.
//   * align_content   -> multi-line only; degrades to a no-op on single
//                        line (mirrors the wrap limitation above).
//   * per-child grow  -> gtk_widget_set_hexpand / vexpand on the main axis.
//
// NOTE: This is intentionally an approximation. A faithful flexbox solver
// (basis/grow/shrink resolution, multi-line wrap, align_content) is a
// follow-up; this v1 maps the flex properties onto GtkBox semantics so the
// container renders and reacts to property changes against real GTK4.

#include "mpapp/handlers/linux/flex_layout_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

// Main axis is horizontal for row/row_reverse, vertical for column*.
bool is_horizontal(flex_direction d) noexcept {
    return d == flex_direction::row || d == flex_direction::row_reverse;
}

// Map justify_content (main-axis distribution) onto a GtkAlign for the box.
// GtkBox has no per-gap distribution, so space_* collapses to FILL and the
// discrete start/center/end map directly.
GtkAlign justify_to_align(flex_justify j) noexcept {
    switch (j) {
        case flex_justify::start:         return GTK_ALIGN_START;
        case flex_justify::center:        return GTK_ALIGN_CENTER;
        case flex_justify::end:           return GTK_ALIGN_END;
        case flex_justify::space_between:  // fallthrough
        case flex_justify::space_around:   // fallthrough
        case flex_justify::space_evenly:  return GTK_ALIGN_FILL;
    }
    return GTK_ALIGN_FILL;
}

// Map align_items (cross-axis alignment of children) onto a GtkAlign.
GtkAlign align_items_to_align(flex_align_items a) noexcept {
    switch (a) {
        case flex_align_items::stretch: return GTK_ALIGN_FILL;
        case flex_align_items::center:  return GTK_ALIGN_CENTER;
        case flex_align_items::start:   return GTK_ALIGN_START;
        case flex_align_items::end:     return GTK_ALIGN_END;
    }
    return GTK_ALIGN_FILL;
}

} // namespace

flex_layout_handler<platform::linux_>::flex_layout_handler() {
    native_ = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(native_), TRUE);
    gtk_widget_set_vexpand(static_cast<GtkWidget*>(native_), TRUE);
}

flex_layout_handler<platform::linux_>::~flex_layout_handler() = default;

// ----- apply_* : push a resolved property value onto the GtkBox -----------

void flex_layout_handler<platform::linux_>::apply_direction(flex_direction d) {
    if (native_ == nullptr) return;
    gtk_orientable_set_orientation(
        GTK_ORIENTABLE(static_cast<GtkWidget*>(native_)),
        is_horizontal(d) ? GTK_ORIENTATION_HORIZONTAL
                         : GTK_ORIENTATION_VERTICAL);
}

void flex_layout_handler<platform::linux_>::apply_wrap(flex_wrap /*w*/) {
    // GtkBox is strictly single-line; true multi-line wrap requires a custom
    // GtkLayoutManager and is a documented follow-up. No-op for v1.
}

void flex_layout_handler<platform::linux_>::apply_justify_content(flex_justify j) {
    if (native_ == nullptr) return;
    GtkWidget* w = static_cast<GtkWidget*>(native_);
    const GtkAlign a = justify_to_align(j);
    // justify_content governs the main axis = the box orientation.
    const bool horizontal =
        gtk_orientable_get_orientation(GTK_ORIENTABLE(w))
            == GTK_ORIENTATION_HORIZONTAL;
    if (horizontal) {
        gtk_widget_set_halign(w, a);
    } else {
        gtk_widget_set_valign(w, a);
    }
}

void flex_layout_handler<platform::linux_>::apply_align_items(flex_align_items a) {
    if (native_ == nullptr) return;
    GtkWidget* w = static_cast<GtkWidget*>(native_);
    const GtkAlign ga = align_items_to_align(a);
    // align_items governs the cross axis: opposite of the box orientation.
    const bool horizontal =
        gtk_orientable_get_orientation(GTK_ORIENTABLE(w))
            == GTK_ORIENTATION_HORIZONTAL;
    // Iterate children and set their cross-axis alignment.
    for (GtkWidget* child = gtk_widget_get_first_child(w);
         child != nullptr;
         child = gtk_widget_get_next_sibling(child)) {
        if (horizontal) {
            gtk_widget_set_valign(child, ga);
        } else {
            gtk_widget_set_halign(child, ga);
        }
    }
}

void flex_layout_handler<platform::linux_>::apply_align_content(flex_align_content /*a*/) {
    // align_content only affects multi-line layouts. GtkBox is single-line,
    // so this degrades to a no-op (see wrap limitation). Follow-up.
}

void flex_layout_handler<platform::linux_>::apply_position(flex_position /*p*/) {
    // Absolute positioning has no GtkBox equivalent; a GtkFixed/GtkOverlay
    // overlay would be required. No-op for the v1 GtkBox mapping. Follow-up.
}

// ----- map_* : seed the current value and subscribe for changes -----------

void flex_layout_handler<platform::linux_>::map_direction(basic_flex_layout& f) {
    apply_direction(f.direction.get());
    f.direction.changed.subscribe(direction_slot_, direction_cb_);
}

void flex_layout_handler<platform::linux_>::map_wrap(basic_flex_layout& f) {
    apply_wrap(f.wrap.get());
    f.wrap.changed.subscribe(wrap_slot_, wrap_cb_);
}

void flex_layout_handler<platform::linux_>::map_justify_content(basic_flex_layout& f) {
    apply_justify_content(f.justify_content.get());
    f.justify_content.changed.subscribe(justify_slot_, justify_cb_);
}

void flex_layout_handler<platform::linux_>::map_align_items(basic_flex_layout& f) {
    apply_align_items(f.align_items.get());
    f.align_items.changed.subscribe(align_items_slot_, align_items_cb_);
}

void flex_layout_handler<platform::linux_>::map_align_content(basic_flex_layout& f) {
    apply_align_content(f.align_content.get());
    f.align_content.changed.subscribe(align_content_slot_, align_content_cb_);
}

void flex_layout_handler<platform::linux_>::map_position(basic_flex_layout& f) {
    apply_position(f.position.get());
    f.position.changed.subscribe(position_slot_, position_cb_);
}

// ----- add_child : append a resolved native widget + per-child props ------

void flex_layout_handler<platform::linux_>::add_child(basic_flex_layout& f, view& child) {
    if (native_ == nullptr) return;

    // ADR-0013: resolve the child's native GtkWidget* via the dispatch
    // registry (same pattern as grid/stack handlers).
    GtkWidget* w = detail::linux_dispatch::dispatch(&child);
    if (w == nullptr) return;

    GtkWidget* box = static_cast<GtkWidget*>(native_);
    gtk_box_append(GTK_BOX(box), w);

    const bool horizontal =
        gtk_orientable_get_orientation(GTK_ORIENTABLE(box))
            == GTK_ORIENTATION_HORIZONTAL;

    // Per-child attached props: grow drives main-axis expand.
    const auto props = f.get_child_props(child);
    if (props.grow > 0.0) {
        if (horizontal) {
            gtk_widget_set_hexpand(w, TRUE);
        } else {
            gtk_widget_set_vexpand(w, TRUE);
        }
    }

    // Cross-axis alignment: align_self overrides the container align_items
    // when it is not `auto_`; otherwise the container value applies.
    const flex_align_items container = f.align_items.get();
    GtkAlign cross;
    switch (props.align_self) {
        case flex_align_self::stretch: cross = GTK_ALIGN_FILL;   break;
        case flex_align_self::center:  cross = GTK_ALIGN_CENTER; break;
        case flex_align_self::start:   cross = GTK_ALIGN_START;  break;
        case flex_align_self::end:     cross = GTK_ALIGN_END;    break;
        case flex_align_self::auto_:   // fallthrough
        default:                       cross = align_items_to_align(container); break;
    }
    if (horizontal) {
        gtk_widget_set_valign(w, cross);
    } else {
        gtk_widget_set_halign(w, cross);
    }
}

// ----- gestures : RFC-0003 controller wiring ------------------------------

void flex_layout_handler<platform::linux_>::map_gestures(basic_flex_layout& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal

// ---------- Self-registration with the per-platform dispatch registry -----
namespace {

GtkWidget* dispatch_flex_layout(::mpapp::view* v) {
    if (auto* f = dynamic_cast<::mpapp::internal::basic_flex_layout*>(v);
        f && f->has_handler()) {
        return GTK_WIDGET(f->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_flex_layout); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
