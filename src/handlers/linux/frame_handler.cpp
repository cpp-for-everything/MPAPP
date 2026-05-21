// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 frame handler implementation. `mpapp::frame` is the
// deprecated MAUI-9 alias for `Border`; kept for one-to-one XAML
// migration parity.

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#endif

#include "mpapp/handlers/linux/frame_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/frame.hpp"
#include "mpapp/view.hpp"

namespace mpapp {

namespace {

std::string next_frame_class() {
    static std::atomic<unsigned> counter{0};
    return "mpapp-frame-" + std::to_string(counter.fetch_add(1));
}

int to255(double v) {
    if (!(v == v)) return 0;
    if (v <= 0.0) return 0;
    if (v >= 1.0) return 255;
    return static_cast<int>(v * 255.0 + 0.5);
}

} // namespace

frame_handler<platform::linux_>::frame_handler() {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    native_ = box;

    class_name_ = next_frame_class();
    gtk_widget_add_css_class(box, class_name_.c_str());

    GtkCssProvider* provider = gtk_css_provider_new();
    provider_ = provider;
    if (GdkDisplay* display = gdk_display_get_default(); display != nullptr) {
        gtk_style_context_add_provider_for_display(
            display, GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
    reload_css();
}

frame_handler<platform::linux_>::~frame_handler() {
    if (provider_ != nullptr) {
        if (GdkDisplay* display = gdk_display_get_default(); display != nullptr) {
            gtk_style_context_remove_provider_for_display(
                display, GTK_STYLE_PROVIDER(static_cast<GtkCssProvider*>(provider_)));
        }
        g_object_unref(provider_);
        provider_ = nullptr;
    }
}

void frame_handler<platform::linux_>::reload_css() {
    if (provider_ == nullptr) return;
    // -1 corner radius means "platform default"; on GTK that is 0.
    const double radius = (cached_corner_radius_ < 0.0f) ? 0.0
                                                         : static_cast<double>(cached_corner_radius_);
    // MAUI Frame draws a 1px border by default.
    const double border_thickness = 1.0;
    // Cheap drop-shadow approximation when has_shadow is set. Keeps parity
    // with MAUI's hard-coded `Radius=5, Opacity=0.8` Frame shadow on iOS.
    const char* shadow_decl = cached_has_shadow_
        ? "box-shadow: 0 2px 5px rgba(0,0,0,0.8);"
        : "box-shadow: none;";
    char buf[768];
    std::snprintf(
        buf, sizeof(buf),
        ".%s { border: %.1fpx solid rgba(%d,%d,%d,%.3f); "
        "border-radius: %.1fpx; "
        "padding: %.1fpx %.1fpx %.1fpx %.1fpx; %s }",
        class_name_.c_str(),
        border_thickness,
        to255(cached_border_color_.r),
        to255(cached_border_color_.g),
        to255(cached_border_color_.b),
        cached_border_color_.a,
        radius,
        cached_padding_.top, cached_padding_.right,
        cached_padding_.bottom, cached_padding_.left,
        shadow_decl);
    gtk_css_provider_load_from_data(
        static_cast<GtkCssProvider*>(provider_), buf, -1);
}

void frame_handler<platform::linux_>::apply_content(const std::shared_ptr<view>& v) {
    GtkBox* box = GTK_BOX(static_cast<GtkWidget*>(native_));
    if (current_child_ != nullptr) {
        gtk_box_remove(box, GTK_WIDGET(current_child_));
        current_child_ = nullptr;
    }
    GtkWidget* child = v ? detail::linux_dispatch::dispatch(v.get()) : nullptr;
    if (child != nullptr) {
        gtk_box_append(box, child);
        current_child_ = child;
    }
}

void frame_handler<platform::linux_>::apply_border_color(const color& c)   { cached_border_color_ = c; reload_css(); }
void frame_handler<platform::linux_>::apply_has_shadow(bool b)             { cached_has_shadow_ = b; reload_css(); }
void frame_handler<platform::linux_>::apply_corner_radius(float r)         { cached_corner_radius_ = r; reload_css(); }
void frame_handler<platform::linux_>::apply_padding(const thickness& t)    { cached_padding_ = t; reload_css(); }

void frame_handler<platform::linux_>::map_content(frame& f) {
    apply_content(f.content.get());
    f.content.changed.subscribe(content_slot_, content_cb_);
}

void frame_handler<platform::linux_>::map_border_color(frame& f) {
    apply_border_color(f.border_color.get());
    f.border_color.changed.subscribe(border_color_slot_, border_color_cb_);
}

void frame_handler<platform::linux_>::map_has_shadow(frame& f) {
    apply_has_shadow(f.has_shadow.get());
    f.has_shadow.changed.subscribe(has_shadow_slot_, has_shadow_cb_);
}

void frame_handler<platform::linux_>::map_corner_radius(frame& f) {
    apply_corner_radius(f.corner_radius.get());
    f.corner_radius.changed.subscribe(corner_radius_slot_, corner_radius_cb_);
}

void frame_handler<platform::linux_>::map_padding(frame& f) {
    apply_padding(f.padding.get());
    f.padding.changed.subscribe(padding_slot_, padding_cb_);
}

void frame_handler<platform::linux_>::bind_content(frame& f, view& child) {
    f.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp

// ---------- Self-registration with the per-platform dispatch registry --

namespace {

GtkWidget* dispatch_frame(::mpapp::view* v) {
    if (auto* fr = dynamic_cast<::mpapp::frame*>(v); fr && fr->has_handler()) {
        return GTK_WIDGET(fr->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_frame); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif
