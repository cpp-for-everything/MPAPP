// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 progress_bar handler implementation.

#include "mpapp/handlers/linux/progress_bar_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <gtk/gtk.h>

namespace mpapp {

namespace {

std::string next_class() {
    static std::atomic<unsigned> counter{0};
    return "mpapp-progress-" + std::to_string(counter.fetch_add(1));
}

struct rgba { double r=0, g=0.4, b=0.85, a=1; };
rgba parse_brush(const brush_ref& br, rgba fallback) {
    const std::string& name = br.name;
    if (name.empty()) return fallback;
    if (name[0] == '#') {
        unsigned long v = std::strtoul(name.c_str() + 1, nullptr, 16);
        auto byte = [](unsigned long x){ return (x & 0xFF) / 255.0; };
        rgba out = fallback;
        if (name.size() == 7) { out.r = byte(v >> 16); out.g = byte(v >> 8); out.b = byte(v); out.a = 1.0; }
        else if (name.size() == 9) { out.a = byte(v >> 24); out.r = byte(v >> 16); out.g = byte(v >> 8); out.b = byte(v); }
        return out;
    }
    if (name == "Red")   return {220/255.0,  50/255.0,  50/255.0, 1};
    if (name == "Green") return { 80/255.0, 180/255.0,  80/255.0, 1};
    if (name == "Blue")  return { 60/255.0, 120/255.0, 220/255.0, 1};
    if (name == "Teal")  return {0, 150/255.0, 165/255.0, 1};
    if (name == "Gray")  return {0.5, 0.5, 0.5, 1};
    return fallback;
}

int to255(double v) {
    if (!(v == v)) return 0;
    if (v <= 0.0) return 0;
    if (v >= 1.0) return 255;
    return static_cast<int>(v * 255.0 + 0.5);
}

} // namespace

progress_bar_handler<platform::linux_>::progress_bar_handler() {
    GtkWidget* bar = gtk_progress_bar_new();
    native_ = bar;

    class_name_ = next_class();
    gtk_widget_add_css_class(bar, class_name_.c_str());

    GtkCssProvider* provider = gtk_css_provider_new();
    provider_ = provider;
    if (GdkDisplay* d = gdk_display_get_default(); d != nullptr) {
        gtk_style_context_add_provider_for_display(
            d, GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}

progress_bar_handler<platform::linux_>::~progress_bar_handler() {
    if (provider_ != nullptr) {
        if (GdkDisplay* d = gdk_display_get_default(); d != nullptr) {
            gtk_style_context_remove_provider_for_display(
                d, GTK_STYLE_PROVIDER(static_cast<GtkCssProvider*>(provider_)));
        }
        g_object_unref(provider_);
        provider_ = nullptr;
    }
}

void progress_bar_handler<platform::linux_>::reload_css() {
    if (provider_ == nullptr) return;
    rgba fg = parse_brush(cached_color_, {0, 0.4, 0.85, 1});
    rgba bg = parse_brush(cached_bg_,    {0.94, 0.94, 0.94, 1});
    char buf[512];
    // GtkProgressBar: the `progress` subnode is the filled portion;
    // the `trough` subnode is the unfilled track.
    std::snprintf(buf, sizeof(buf),
        ".%s > trough > progress { background-color: rgba(%d,%d,%d,%.3f); background-image: none; } "
        ".%s > trough { background-color: rgba(%d,%d,%d,%.3f); background-image: none; }",
        class_name_.c_str(),
        to255(fg.r), to255(fg.g), to255(fg.b), fg.a,
        class_name_.c_str(),
        to255(bg.r), to255(bg.g), to255(bg.b), bg.a);
    gtk_css_provider_load_from_data(
        static_cast<GtkCssProvider*>(provider_), buf, -1);
}

void progress_bar_handler<platform::linux_>::apply_progress(double v) {
    if (v < 0) v = 0; if (v > 1) v = 1;
    gtk_progress_bar_set_fraction(
        GTK_PROGRESS_BAR(static_cast<GtkWidget*>(native_)), v);
}

void progress_bar_handler<platform::linux_>::apply_color(const brush_ref& b)            { cached_color_ = b; reload_css(); }
void progress_bar_handler<platform::linux_>::apply_background_color(const brush_ref& b) { cached_bg_    = b; reload_css(); }

void progress_bar_handler<platform::linux_>::map_progress(progress_bar& p) {
    apply_progress(p.progress.get());
    p.progress.changed.subscribe(progress_slot_, progress_cb_);
}
void progress_bar_handler<platform::linux_>::map_color(progress_bar& p) {
    apply_color(p.color.get());
    p.color.changed.subscribe(color_slot_, color_cb_);
}
void progress_bar_handler<platform::linux_>::map_background_color(progress_bar& p) {
    apply_background_color(p.background_color.get());
    p.background_color.changed.subscribe(bg_slot_, bg_cb_);
}

} // namespace mpapp


// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register progress_bar so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/progress_bar.hpp"

namespace {

GtkWidget* dispatch_progress_bar(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::progress_bar*>(v); w && w->has_handler()) {
        return GTK_WIDGET(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_progress_bar); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
