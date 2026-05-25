// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_activity_indicator handler implementation.

#include "mpapp/handlers/linux/activity_indicator_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <gtk/gtk.h>

namespace mpapp::internal {

namespace {

std::string next_class() {
    static std::atomic<unsigned> counter{0};
    return "mpapp-spinner-" + std::to_string(counter.fetch_add(1));
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
    if (name == "Red")   return {220/255.0,  50/255.0,  50/255.0, 1};
    if (name == "Green") return { 80/255.0, 180/255.0,  80/255.0, 1};
    if (name == "Blue")  return { 60/255.0, 120/255.0, 220/255.0, 1};
    if (name == "Teal")  return {0, 150/255.0, 165/255.0, 1};
    return out;
}

int to255(double v) {
    if (!(v == v)) return 0;
    if (v <= 0.0) return 0;
    if (v >= 1.0) return 255;
    return static_cast<int>(v * 255.0 + 0.5);
}

} // namespace

activity_indicator_handler<platform::linux_>::activity_indicator_handler() {
    GtkWidget* spin = gtk_spinner_new();
    native_ = spin;

    class_name_ = next_class();
    gtk_widget_add_css_class(spin, class_name_.c_str());

    GtkCssProvider* provider = gtk_css_provider_new();
    provider_ = provider;
    if (GdkDisplay* d = gdk_display_get_default(); d != nullptr) {
        gtk_style_context_add_provider_for_display(
            d, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}

activity_indicator_handler<platform::linux_>::~activity_indicator_handler() {
    if (provider_ != nullptr) {
        if (GdkDisplay* d = gdk_display_get_default(); d != nullptr) {
            gtk_style_context_remove_provider_for_display(
                d, GTK_STYLE_PROVIDER(static_cast<GtkCssProvider*>(provider_)));
        }
        g_object_unref(provider_);
        provider_ = nullptr;
    }
}

void activity_indicator_handler<platform::linux_>::apply_is_running(bool v) {
    GtkSpinner* sp = GTK_SPINNER(static_cast<GtkWidget*>(native_));
    if (v) gtk_spinner_start(sp);
    else   gtk_spinner_stop(sp);
    gtk_widget_set_visible(GTK_WIDGET(sp), v);
}

void activity_indicator_handler<platform::linux_>::apply_color(const brush_ref& b) {
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

void activity_indicator_handler<platform::linux_>::map_is_running(basic_activity_indicator& a) {
    apply_is_running(a.is_running.get());
    a.is_running.changed.subscribe(is_running_slot_, is_running_cb_);
}

void activity_indicator_handler<platform::linux_>::map_color(basic_activity_indicator& a) {
    apply_color(a.color.get());
    a.color.changed.subscribe(color_slot_, color_cb_);
}

void activity_indicator_handler<platform::linux_>::map_gestures(basic_activity_indicator& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_activity_indicator so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/internal/basic_activity_indicator.hpp"

namespace {

GtkWidget* dispatch_activity_indicator(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_activity_indicator*>(v); w && w->has_handler()) {
        return GTK_WIDGET(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_activity_indicator); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
