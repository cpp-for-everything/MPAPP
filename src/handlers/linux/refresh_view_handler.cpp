// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_refresh_view handler implementation.

#include "mpapp/handlers/linux/refresh_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

std::string next_class() {
    static std::atomic<unsigned> counter{0};
    return "mpapp-refresh-" + std::to_string(counter.fetch_add(1));
}

struct rgba { double r=0, g=0.4, b=0.85, a=1; };

rgba parse_brush(const brush_ref& br) {
    rgba out;
    const std::string& name = br.name;
    if (name.empty()) return out;
    if (name[0] == '#') {
        unsigned long v = std::strtoul(name.c_str() + 1, nullptr, 16);
        auto byte = [](unsigned long x){ return (x & 0xFF) / 255.0; };
        if (name.size() == 7) {
            out.r = byte(v >> 16); out.g = byte(v >> 8); out.b = byte(v); out.a = 1.0;
        } else if (name.size() == 9) {
            out.a = byte(v >> 24); out.r = byte(v >> 16); out.g = byte(v >> 8); out.b = byte(v);
        }
        return out;
    }
    if (name == "Red")        return {220/255.0,  50/255.0,  50/255.0, 1};
    if (name == "Green")      return { 80/255.0, 180/255.0,  80/255.0, 1};
    if (name == "Blue")       return { 60/255.0, 120/255.0, 220/255.0, 1};
    if (name == "Teal")       return {0, 150/255.0, 165/255.0, 1};
    if (name == "Black")      return {0, 0, 0, 1};
    if (name == "White")      return {1, 1, 1, 1};
    if (name == "Gray")       return {128/255.0, 128/255.0, 128/255.0, 1};
    if (name == "DodgerBlue") return {30/255.0, 144/255.0, 255/255.0, 1};
    return out;
}

int to255(double v) {
    if (!(v == v)) return 0;
    if (v <= 0.0) return 0;
    if (v >= 1.0) return 255;
    return static_cast<int>(v * 255.0 + 0.5);
}

} // namespace

refresh_view_handler<platform::linux_>::refresh_view_handler() {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    native_ = box;

    GtkWidget* spin = gtk_spinner_new();
    spinner_ = spin;
    gtk_widget_set_halign(spin, GTK_ALIGN_CENTER);
    gtk_widget_set_visible(spin, FALSE);   // hidden by default
    gtk_box_append(GTK_BOX(box), spin);

    // Per-handler CSS provider so the spinner colour can be tinted via a
    // unique class name. Mirrors basic_activity_indicator's approach.
    class_name_ = next_class();
    gtk_widget_add_css_class(spin, class_name_.c_str());
    GtkCssProvider* provider = gtk_css_provider_new();
    provider_ = provider;
    if (GdkDisplay* d = gdk_display_get_default(); d != nullptr) {
        gtk_style_context_add_provider_for_display(
            d, GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}

refresh_view_handler<platform::linux_>::~refresh_view_handler() {
    if (provider_ != nullptr) {
        if (GdkDisplay* d = gdk_display_get_default(); d != nullptr) {
            gtk_style_context_remove_provider_for_display(
                d, GTK_STYLE_PROVIDER(static_cast<GtkCssProvider*>(provider_)));
        }
        g_object_unref(provider_);
        provider_ = nullptr;
    }
}

void refresh_view_handler<platform::linux_>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    GtkBox* box = GTK_BOX(static_cast<GtkWidget*>(native_));

    if (current_child_ != nullptr) {
        gtk_box_remove(box, GTK_WIDGET(current_child_));
        current_child_ = nullptr;
    }

    if (!v) return;
    // ADR-0013 registry first; if no widget is registered for the child
    // type, leave content empty (per the M-04b worker-prompt's
    // "easiest for now" simplification).
    if (GtkWidget* child = detail::linux_dispatch::dispatch(v.get()); child != nullptr) {
        gtk_box_append(box, child);
        current_child_ = child;
    }
}

void refresh_view_handler<platform::linux_>::apply_is_refreshing(bool v) {
    if (spinner_ == nullptr) return;
    GtkSpinner* sp = GTK_SPINNER(static_cast<GtkWidget*>(spinner_));
    if (v) gtk_spinner_start(sp);
    else   gtk_spinner_stop(sp);
    gtk_widget_set_visible(GTK_WIDGET(sp), v);
}

void refresh_view_handler<platform::linux_>::apply_refresh_color(const brush_ref& b) {
    if (provider_ == nullptr) return;
    rgba c = parse_brush(b);
    char buf[256];
    std::snprintf(buf, sizeof(buf),
        ".%s { color: rgba(%d,%d,%d,%.3f); }",
        class_name_.c_str(),
        to255(c.r), to255(c.g), to255(c.b), c.a);
    gtk_css_provider_load_from_data(
        static_cast<GtkCssProvider*>(provider_), buf, -1);
}

void refresh_view_handler<platform::linux_>::map_content(basic_refresh_view& r) {
    apply_content(r.content.get());
    r.content.changed.subscribe(content_slot_, content_cb_);
}

void refresh_view_handler<platform::linux_>::map_is_refreshing(basic_refresh_view& r) {
    apply_is_refreshing(r.is_refreshing.get());
    r.is_refreshing.changed.subscribe(is_refreshing_slot_, is_refreshing_cb_);
}

void refresh_view_handler<platform::linux_>::map_refresh_color(basic_refresh_view& r) {
    apply_refresh_color(r.refresh_color.get());
    r.refresh_color.changed.subscribe(refresh_color_slot_, refresh_color_cb_);
}

void refresh_view_handler<platform::linux_>::bind_content(basic_refresh_view& r, view& child) {
    r.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

// ----- ADR-0013 self-registration --------------------------------------

namespace {

GtkWidget* dispatch_refresh_view(::mpapp::view* v) {
    if (auto* r = dynamic_cast<::mpapp::internal::basic_refresh_view*>(v); r && r->has_handler()) {
        return GTK_WIDGET(r->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_refresh_view);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
