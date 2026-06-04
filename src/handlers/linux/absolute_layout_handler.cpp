// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_absolute_layout handler implementation.
//
// GtkFixed is the native analogue to MAUI's AbsoluteLayout: children are
// placed at explicit (x, y) via gtk_fixed_put / gtk_fixed_move and sized
// via gtk_widget_set_size_request. GtkFixed has no proportional placement,
// so proportional layout_flags are resolved here against the container's
// current allocation (gtk_widget_get_width / gtk_widget_get_height) before
// the native move/size call.

#include "mpapp/handlers/linux/absolute_layout_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

constexpr bool has_flag(absolute_layout_flags f, absolute_layout_flags bit) noexcept {
    return (static_cast<std::uint8_t>(f) & static_cast<std::uint8_t>(bit)) != 0;
}

} // namespace

absolute_layout_handler<platform::linux_>::absolute_layout_handler() {
    native_ = gtk_fixed_new();
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(native_), TRUE);
    gtk_widget_set_vexpand(static_cast<GtkWidget*>(native_), TRUE);
}

absolute_layout_handler<platform::linux_>::~absolute_layout_handler() = default;

void absolute_layout_handler<platform::linux_>::apply_bounds(view& child,
                                                             const rect& r,
                                                             absolute_layout_flags f) {
    if (native_ == nullptr) return;

    GtkWidget* w = detail::linux_dispatch::dispatch(&child);
    if (w == nullptr) return;

    auto* container = static_cast<GtkWidget*>(native_);

    // Resolve proportional components against the container's current
    // allocation. A proportional value is a 0..1 fraction of the matching
    // container extent; otherwise it is an absolute device-independent pixel.
    const double cw = static_cast<double>(gtk_widget_get_width(container));
    const double ch = static_cast<double>(gtk_widget_get_height(container));

    const double x = has_flag(f, absolute_layout_flags::x_proportional)      ? r.x * cw      : r.x;
    const double y = has_flag(f, absolute_layout_flags::y_proportional)      ? r.y * ch      : r.y;
    const double width  = has_flag(f, absolute_layout_flags::width_proportional)  ? r.width * cw  : r.width;
    const double height = has_flag(f, absolute_layout_flags::height_proportional) ? r.height * ch : r.height;

    gtk_fixed_move(GTK_FIXED(container), w, x, y);

    if (width > 0.0 || height > 0.0) {
        gtk_widget_set_size_request(w,
                                    width  > 0.0 ? static_cast<int>(width)  : -1,
                                    height > 0.0 ? static_cast<int>(height) : -1);
    }
}

void absolute_layout_handler<platform::linux_>::add_child(basic_absolute_layout& a, view& child) {
    if (native_ == nullptr) return;

    GtkWidget* w = detail::linux_dispatch::dispatch(&child);
    if (w == nullptr) return;

    const rect                  r = a.get_layout_bounds(child);
    const absolute_layout_flags f = a.get_layout_flags(child);

    auto* container = static_cast<GtkWidget*>(native_);

    // Initial placement uses the absolute x/y; proportional components are
    // resolved against the (possibly zero, pre-allocation) container size.
    const double cw = static_cast<double>(gtk_widget_get_width(container));
    const double ch = static_cast<double>(gtk_widget_get_height(container));

    const double x = has_flag(f, absolute_layout_flags::x_proportional) ? r.x * cw : r.x;
    const double y = has_flag(f, absolute_layout_flags::y_proportional) ? r.y * ch : r.y;

    gtk_fixed_put(GTK_FIXED(container), w, x, y);

    const double width  = has_flag(f, absolute_layout_flags::width_proportional)  ? r.width * cw  : r.width;
    const double height = has_flag(f, absolute_layout_flags::height_proportional) ? r.height * ch : r.height;
    if (width > 0.0 || height > 0.0) {
        gtk_widget_set_size_request(w,
                                    width  > 0.0 ? static_cast<int>(width)  : -1,
                                    height > 0.0 ? static_cast<int>(height) : -1);
    }
}

void absolute_layout_handler<platform::linux_>::map_layout_bounds(basic_absolute_layout& a, view& child) {
    apply_bounds(child, a.get_layout_bounds(child), a.get_layout_flags(child));
}

void absolute_layout_handler<platform::linux_>::map_layout_flags(basic_absolute_layout& a, view& child) {
    apply_bounds(child, a.get_layout_bounds(child), a.get_layout_flags(child));
}

void absolute_layout_handler<platform::linux_>::map_gestures(basic_absolute_layout& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal

// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_absolute_layout(::mpapp::view* v) {
    if (auto* a = dynamic_cast<::mpapp::internal::basic_absolute_layout*>(v); a && a->has_handler()) {
        return GTK_WIDGET(a->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_absolute_layout); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
