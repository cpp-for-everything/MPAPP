// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_flex_layout handler implementation.
//
// GTK4 ships no native flexbox container. This implementation drives the
// neutral, platform-independent `mpapp::flex_arrange` solver to real GTK4
// pixels via a custom GtkLayoutManager subclass:
//
//   * native_ is a plain GtkWidget whose layout manager is an
//     `MpappFlexLayout` (defined below). The manager holds a borrowed
//     pointer to the owning `basic_flex_layout` surface.
//   * The manager's `measure` vfunc reports a natural size: along the main
//     axis it sums children natural sizes + gaps, on the cross axis it
//     takes the max child natural size.
//   * The manager's `allocate` vfunc builds a `flex_container_input` from
//     the live surface Observables (direction / wrap / justify_content /
//     align_items / align_content) plus the allocated width/height, then
//     gathers a `flex_item_input` per surface child (resolving each child's
//     GtkWidget* via the dispatch registry and measuring it for natural
//     main/cross sizes), runs `mpapp::flex_arrange`, and finally calls
//     gtk_widget_size_allocate() per computed rect.
//
// Property changes flow through the handler's apply_* methods which simply
// queue a relayout (gtk_widget_queue_allocate) — the manager re-reads the
// live Observable values on the next allocate pass, so there is no derived
// state to keep in sync.

#include "mpapp/handlers/linux/flex_layout_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include <vector>

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/layout/flex_arrange.hpp"

// ===========================================================================
// Custom GtkLayoutManager that drives mpapp::flex_arrange.
//
// File-scope GObject type: not exported, lives only in this TU. Stores a
// borrowed pointer to the basic_flex_layout surface; the handler sets it at
// construction time. The manager never owns the surface.
// ===========================================================================
namespace {

struct _MpappFlexLayout {
    GtkLayoutManager parent_instance;
    // Borrowed: owned by the application's flex_layout / handler. Never freed
    // here. May be null transiently before the handler binds it.
    ::mpapp::internal::basic_flex_layout* surface;
};

} // namespace

// G_DECLARE_FINAL_TYPE emits the MPAPP_FLEX_LAYOUT() cast + type accessors at
// file scope so they are usable from both anonymous-namespace code and the
// mpapp::internal handler methods below.
G_DECLARE_FINAL_TYPE(MpappFlexLayout, mpapp_flex_layout, MPAPP, FLEX_LAYOUT,
                     GtkLayoutManager)

namespace {

// G_DEFINE_TYPE forward-declares mpapp_flex_layout_init / _class_init and the
// generated registration glue. Keeping it inside this anonymous namespace
// means those forward declarations and the definitions below share one scope.
G_DEFINE_TYPE(MpappFlexLayout, mpapp_flex_layout, GTK_TYPE_LAYOUT_MANAGER)

// Measure one GtkWidget's natural size on the given orientation.
int natural_for(GtkWidget* w, GtkOrientation orientation) {
    int minimum = 0;
    int natural = 0;
    gtk_widget_measure(w, orientation, /*for_size=*/-1, &minimum, &natural,
                       nullptr, nullptr);
    return natural;
}

// ----- measure vfunc -------------------------------------------------------
//
// Reports the container's natural extent: along the main axis the sum of
// child natural sizes (no inter-item gaps — the surface exposes none), and
// on the cross axis the max child natural size. The minimum equals the
// natural for this simple report.
void mpapp_flex_layout_measure(GtkLayoutManager* manager,
                               GtkWidget*        /*widget*/,
                               GtkOrientation    orientation,
                               int               /*for_size*/,
                               int*              minimum,
                               int*              natural,
                               int*              minimum_baseline,
                               int*              natural_baseline) {
    auto* self = MPAPP_FLEX_LAYOUT(manager);

    int main_sum = 0;
    int cross_max = 0;

    if (self->surface != nullptr) {
        const bool row = ::mpapp::internal::flex::is_row(
            self->surface->direction.get());
        // The orientation we are asked to measure maps onto either the main
        // or cross axis depending on the flex direction.
        const GtkOrientation main_orientation =
            row ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
        const GtkOrientation cross_orientation =
            row ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;

        const std::size_t count = self->surface->child_count();
        for (std::size_t i = 0; i < count; ++i) {
            ::mpapp::view* child = self->surface->child_at(i);
            if (child == nullptr) continue;
            GtkWidget* cw = ::mpapp::detail::linux_dispatch::dispatch(child);
            if (cw == nullptr || !gtk_widget_should_layout(cw)) continue;

            main_sum += natural_for(cw, main_orientation);
            const int c = natural_for(cw, cross_orientation);
            if (c > cross_max) cross_max = c;
        }

        const int requested =
            (orientation == main_orientation) ? main_sum : cross_max;
        if (minimum != nullptr) *minimum = requested;
        if (natural != nullptr) *natural = requested;
    } else {
        if (minimum != nullptr) *minimum = 0;
        if (natural != nullptr) *natural = 0;
    }

    if (minimum_baseline != nullptr) *minimum_baseline = -1;
    if (natural_baseline != nullptr) *natural_baseline = -1;
}

// ----- allocate vfunc ------------------------------------------------------
//
// Builds the flex_container_input + per-child flex_item_input from the live
// surface, runs mpapp::flex_arrange, and pushes the resulting rectangles to
// GTK via gtk_widget_size_allocate().
void mpapp_flex_layout_allocate(GtkLayoutManager* manager,
                                GtkWidget*        /*widget*/,
                                int               width,
                                int               height,
                                int               /*baseline*/) {
    auto* self = MPAPP_FLEX_LAYOUT(manager);
    if (self->surface == nullptr) return;

    ::mpapp::internal::basic_flex_layout& surface = *self->surface;

    // Container geometry + flex properties, straight from the live surface.
    ::mpapp::flex_container_input container;
    container.width           = static_cast<double>(width);
    container.height          = static_cast<double>(height);
    container.direction       = surface.direction.get();
    container.wrap            = surface.wrap.get();
    container.justify_content = surface.justify_content.get();
    container.align_items     = surface.align_items.get();
    container.align_content   = surface.align_content.get();
    // The mock surface exposes no gap observables; gaps default to 0.
    container.main_gap        = 0.0;
    container.cross_gap       = 0.0;

    const bool row = ::mpapp::internal::flex::is_row(container.direction);
    const GtkOrientation main_orientation =
        row ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
    const GtkOrientation cross_orientation =
        row ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;

    // Gather inputs, keeping a parallel list of the GtkWidget* to allocate.
    const std::size_t count = surface.child_count();
    std::vector<::mpapp::flex_item_input> items;
    std::vector<GtkWidget*>               widgets;
    items.reserve(count);
    widgets.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        ::mpapp::view* child = surface.child_at(i);
        if (child == nullptr) continue;
        GtkWidget* cw = ::mpapp::detail::linux_dispatch::dispatch(child);
        if (cw == nullptr || !gtk_widget_should_layout(cw)) continue;

        const auto props = surface.get_child_props(*child);

        ::mpapp::flex_item_input it;
        it.basis          = props.basis;   // -1 == auto -> use measured_main
        it.grow           = props.grow;
        it.shrink         = props.shrink;
        it.align_self     = props.align_self;
        it.order          = props.order;
        it.measured_main  =
            static_cast<double>(natural_for(cw, main_orientation));
        it.measured_cross =
            static_cast<double>(natural_for(cw, cross_orientation));

        items.push_back(it);
        widgets.push_back(cw);
    }

    if (items.empty()) return;

    // Run the neutral solver.
    const std::vector<::mpapp::flex_rect> rects =
        ::mpapp::flex_arrange(container, items);

    // Push the computed rectangles to GTK. `rects` is parallel to `items`,
    // which is parallel to `widgets`.
    for (std::size_t i = 0; i < widgets.size() && i < rects.size(); ++i) {
        const ::mpapp::flex_rect& r = rects[i];
        const GtkAllocation alloc{
            /*.x      =*/ static_cast<int>(r.x),
            /*.y      =*/ static_cast<int>(r.y),
            /*.width  =*/ static_cast<int>(r.width),
            /*.height =*/ static_cast<int>(r.height),
        };
        gtk_widget_size_allocate(widgets[i], &alloc, -1);
    }
}

// ----- GObject boilerplate -------------------------------------------------

void mpapp_flex_layout_class_init(MpappFlexLayoutClass* klass) {
    GtkLayoutManagerClass* lm = GTK_LAYOUT_MANAGER_CLASS(klass);
    lm->measure  = mpapp_flex_layout_measure;
    lm->allocate = mpapp_flex_layout_allocate;
}

void mpapp_flex_layout_init(MpappFlexLayout* self) {
    self->surface = nullptr;
}

// Construct a manager bound to the given flex surface.
GtkLayoutManager* mpapp_flex_layout_new(
    ::mpapp::internal::basic_flex_layout* surface) {
    auto* self = MPAPP_FLEX_LAYOUT(
        g_object_new(mpapp_flex_layout_get_type(), nullptr));
    self->surface = surface;
    return GTK_LAYOUT_MANAGER(self);
}

// Rebind the surface pointer on an existing manager (used by map_* so the
// manager always reflects the surface the handler is wired to).
void mpapp_flex_layout_set_surface(
    GtkLayoutManager*                      manager,
    ::mpapp::internal::basic_flex_layout*  surface) {
    if (manager == nullptr) return;
    MPAPP_FLEX_LAYOUT(manager)->surface = surface;
}

} // namespace

namespace mpapp::internal {

flex_layout_handler<platform::linux_>::flex_layout_handler() {
    // A plain box widget hosts the custom layout manager. The orientation of
    // the box itself is irrelevant — the MpappFlexLayout manager owns all
    // measure/allocate decisions — but a GtkBox gives us gtk_box_append for
    // child parenting and a concrete container to attach the manager to.
    native_ = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* w = static_cast<GtkWidget*>(native_);
    gtk_widget_set_hexpand(w, TRUE);
    gtk_widget_set_vexpand(w, TRUE);
    // Manager constructed without a surface; map_* binds it once the surface
    // is known. Owned by the widget once set.
    gtk_widget_set_layout_manager(w, mpapp_flex_layout_new(nullptr));
}

flex_layout_handler<platform::linux_>::~flex_layout_handler() = default;

// Bind (or rebind) the layout manager's surface pointer to `f`. Idempotent.
void flex_layout_handler<platform::linux_>::bind_surface(basic_flex_layout& f) {
    if (native_ == nullptr) return;
    GtkLayoutManager* lm =
        gtk_widget_get_layout_manager(static_cast<GtkWidget*>(native_));
    mpapp_flex_layout_set_surface(lm, &f);
}

// Queue a fresh solve pass on the container.
void flex_layout_handler<platform::linux_>::queue_relayout() {
    if (native_ == nullptr) return;
    gtk_widget_queue_allocate(static_cast<GtkWidget*>(native_));
}

// ----- apply_* : a live property changed; re-solve on the next pass --------
//
// The manager reads the live Observable values during allocate, so apply_*
// only needs to invalidate the current layout. No derived GTK state to push.

void flex_layout_handler<platform::linux_>::apply_direction(flex_direction /*d*/) {
    queue_relayout();
}

void flex_layout_handler<platform::linux_>::apply_wrap(flex_wrap /*w*/) {
    queue_relayout();
}

void flex_layout_handler<platform::linux_>::apply_justify_content(flex_justify /*j*/) {
    queue_relayout();
}

void flex_layout_handler<platform::linux_>::apply_align_items(flex_align_items /*a*/) {
    queue_relayout();
}

void flex_layout_handler<platform::linux_>::apply_align_content(flex_align_content /*a*/) {
    queue_relayout();
}

void flex_layout_handler<platform::linux_>::apply_position(flex_position /*p*/) {
    // Absolute positioning is solved on the cross/main axes by flex_arrange's
    // relative model; per-child absolute offset is a follow-up. Re-solve so
    // the container reflects any geometry that did change.
    queue_relayout();
}

// ----- map_* : bind the surface, seed the value, subscribe for changes -----

void flex_layout_handler<platform::linux_>::map_direction(basic_flex_layout& f) {
    bind_surface(f);
    apply_direction(f.direction.get());
    f.direction.changed.subscribe(direction_slot_, direction_cb_);
}

void flex_layout_handler<platform::linux_>::map_wrap(basic_flex_layout& f) {
    bind_surface(f);
    apply_wrap(f.wrap.get());
    f.wrap.changed.subscribe(wrap_slot_, wrap_cb_);
}

void flex_layout_handler<platform::linux_>::map_justify_content(basic_flex_layout& f) {
    bind_surface(f);
    apply_justify_content(f.justify_content.get());
    f.justify_content.changed.subscribe(justify_slot_, justify_cb_);
}

void flex_layout_handler<platform::linux_>::map_align_items(basic_flex_layout& f) {
    bind_surface(f);
    apply_align_items(f.align_items.get());
    f.align_items.changed.subscribe(align_items_slot_, align_items_cb_);
}

void flex_layout_handler<platform::linux_>::map_align_content(basic_flex_layout& f) {
    bind_surface(f);
    apply_align_content(f.align_content.get());
    f.align_content.changed.subscribe(align_content_slot_, align_content_cb_);
}

void flex_layout_handler<platform::linux_>::map_position(basic_flex_layout& f) {
    bind_surface(f);
    apply_position(f.position.get());
    f.position.changed.subscribe(position_slot_, position_cb_);
}

// ----- add_child : parent the resolved native widget + queue a relayout ----

void flex_layout_handler<platform::linux_>::add_child(basic_flex_layout& f, view& child) {
    if (native_ == nullptr) return;
    bind_surface(f);

    // ADR-0013: resolve the child's native GtkWidget* via the dispatch
    // registry (same pattern as grid/stack handlers).
    GtkWidget* w = detail::linux_dispatch::dispatch(&child);
    if (w == nullptr) return;

    GtkWidget* box = static_cast<GtkWidget*>(native_);
    gtk_box_append(GTK_BOX(box), w);

    // The custom manager performs measure + allocate from the surface's child
    // collection; parenting alone is enough. Queue a solve so the new child
    // is placed on the next allocate pass.
    queue_relayout();
}

// ----- gestures : RFC-0003 controller wiring ------------------------------

void flex_layout_handler<platform::linux_>::map_gestures(basic_flex_layout& x) {
    if (native_ == nullptr) return;
    bind_surface(x);
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
