// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 box_view handler implementation.

#include "mpapp/handlers/linux/box_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <atomic>
#include <cstdio>

#include <gtk/gtk.h>

namespace mpapp {

namespace {

std::string next_class_name() {
    static std::atomic<unsigned> counter{0};
    return "mpapp-box-" + std::to_string(counter.fetch_add(1));
}

int to_255(double v) {
    if (!(v == v)) return 0;
    if (v <= 0.0)  return 0;
    if (v >= 1.0)  return 255;
    return static_cast<int>(v * 255.0 + 0.5);
}

} // namespace

box_view_handler<platform::linux_>::box_view_handler() {
    GtkWidget* widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    native_ = widget;
    // MAUI BoxView default measured size is 40x40 dip.
    gtk_widget_set_size_request(widget, 40, 40);

    class_name_ = next_class_name();
    gtk_widget_add_css_class(widget, class_name_.c_str());

    GtkCssProvider* provider = gtk_css_provider_new();
    provider_ = provider;
    GdkDisplay* display = gdk_display_get_default();
    if (display != nullptr) {
        gtk_style_context_add_provider_for_display(
            display,
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
    reload_css();
}

box_view_handler<platform::linux_>::~box_view_handler() {
    if (provider_ != nullptr) {
        GdkDisplay* display = gdk_display_get_default();
        if (display != nullptr) {
            gtk_style_context_remove_provider_for_display(
                display, GTK_STYLE_PROVIDER(static_cast<GtkCssProvider*>(provider_)));
        }
        g_object_unref(provider_);
        provider_ = nullptr;
    }
}

void box_view_handler<platform::linux_>::reload_css() {
    if (provider_ == nullptr) return;
    char buf[256];
    std::snprintf(
        buf, sizeof(buf),
        ".%s { background-color: rgba(%d,%d,%d,%.3f); "
        "border-radius: %.1fpx %.1fpx %.1fpx %.1fpx; "
        "padding: 0; }",
        class_name_.c_str(),
        to_255(cached_fill_.r), to_255(cached_fill_.g), to_255(cached_fill_.b),
        cached_fill_.a,
        cached_corners_.top_left, cached_corners_.top_right,
        cached_corners_.bottom_right, cached_corners_.bottom_left);
    gtk_css_provider_load_from_data(
        static_cast<GtkCssProvider*>(provider_), buf, -1);
}

void box_view_handler<platform::linux_>::apply_fill(const color& c) {
    cached_fill_ = c;
    reload_css();
}

void box_view_handler<platform::linux_>::apply_corners(const corner_radius& r) {
    cached_corners_ = r;
    reload_css();
}

void box_view_handler<platform::linux_>::map_fill(box_view& b) {
    apply_fill(b.fill.get());
    b.fill.changed.subscribe(fill_slot_, fill_cb_);
}

void box_view_handler<platform::linux_>::map_corners(box_view& b) {
    apply_corners(b.corners.get());
    b.corners.changed.subscribe(corners_slot_, corners_cb_);
}

} // namespace mpapp


// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register box_view so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/box_view.hpp"

namespace {

GtkWidget* dispatch_box_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::box_view*>(v); w && w->has_handler()) {
        return GTK_WIDGET(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_box_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
