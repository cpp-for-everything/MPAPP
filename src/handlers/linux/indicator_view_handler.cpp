// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_indicator_view handler implementation.

#include "mpapp/handlers/linux/indicator_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

constexpr int kDotSize    = 8;
constexpr int kDotSpacing = 4;

std::string next_iv_class() {
    static std::atomic<unsigned> counter{0};
    return "mpapp-iv-" + std::to_string(counter.fetch_add(1));
}

struct rgba { double r=0, g=0, b=0, a=1; };

rgba parse_brush(const brush_ref& br, rgba fallback) {
    const std::string& name = br.name;
    if (name.empty()) return fallback;
    if (name[0] == '#') {
        unsigned long v = std::strtoul(name.c_str() + 1, nullptr, 16);
        auto byte = [](unsigned long x){ return (x & 0xFF) / 255.0; };
        rgba out = fallback;
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
    if (name == "Black") return {0, 0, 0, 1};
    if (name == "White") return {1, 1, 1, 1};
    if (name == "Gray")  return {160/255.0, 160/255.0, 160/255.0, 1};
    if (name == "Teal")  return {0, 150/255.0, 165/255.0, 1};
    return fallback;
}

int to255(double v) {
    if (!(v == v)) return 0;
    if (v <= 0.0) return 0;
    if (v >= 1.0) return 255;
    return static_cast<int>(v * 255.0 + 0.5);
}

} // namespace

indicator_view_handler<platform::linux_>::indicator_view_handler() {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, kDotSpacing);
    native_ = box;

    base_class_ = next_iv_class();

    GtkCssProvider* provider = gtk_css_provider_new();
    provider_ = provider;
    if (GdkDisplay* d = gdk_display_get_default(); d != nullptr) {
        gtk_style_context_add_provider_for_display(
            d, GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}

indicator_view_handler<platform::linux_>::~indicator_view_handler() {
    if (provider_ != nullptr) {
        if (GdkDisplay* d = gdk_display_get_default(); d != nullptr) {
            gtk_style_context_remove_provider_for_display(
                d, GTK_STYLE_PROVIDER(static_cast<GtkCssProvider*>(provider_)));
        }
        g_object_unref(provider_);
        provider_ = nullptr;
    }
}

void indicator_view_handler<platform::linux_>::rebuild_dots() {
    GtkWidget* box = static_cast<GtkWidget*>(native_);
    if (box == nullptr) return;

    // Remove all existing children.
    GtkWidget* child = gtk_widget_get_first_child(box);
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(box), child);
        child = next;
    }

    // Add `cached_count_` empty labels styled as dots. The selected one
    // gets an extra ".sel" class, the rest get ".dot". We use GtkLabel
    // because it accepts CSS background-color and a fixed size_request.
    for (int i = 0; i < cached_count_; ++i) {
        GtkWidget* dot = gtk_label_new("");
        gtk_widget_set_size_request(dot, kDotSize, kDotSize);
        gtk_widget_add_css_class(dot, base_class_.c_str());
        if (i == cached_position_) {
            gtk_widget_add_css_class(dot, "sel");
        } else {
            gtk_widget_add_css_class(dot, "dot");
        }
        gtk_box_append(GTK_BOX(box), dot);
    }
    reload_css();
}

void indicator_view_handler<platform::linux_>::reload_css() {
    if (provider_ == nullptr) return;
    rgba un  = parse_brush(cached_color_,    {200/255.0, 200/255.0, 200/255.0, 1});
    rgba sel = parse_brush(cached_selected_, { 60/255.0, 120/255.0, 220/255.0, 1});
    char buf[512];
    std::snprintf(buf, sizeof(buf),
        ".%s.dot { background-color: rgba(%d,%d,%d,%.3f); border-radius: %dpx; min-width: %dpx; min-height: %dpx; } "
        ".%s.sel { background-color: rgba(%d,%d,%d,%.3f); border-radius: %dpx; min-width: %dpx; min-height: %dpx; }",
        base_class_.c_str(),
        to255(un.r),  to255(un.g),  to255(un.b),  un.a,
        kDotSize / 2, kDotSize, kDotSize,
        base_class_.c_str(),
        to255(sel.r), to255(sel.g), to255(sel.b), sel.a,
        kDotSize / 2, kDotSize, kDotSize);
    gtk_css_provider_load_from_data(
        static_cast<GtkCssProvider*>(provider_), buf, -1);
}

void indicator_view_handler<platform::linux_>::apply_count(int v) {
    if (v < 0) v = 0;
    cached_count_ = v;
    rebuild_dots();
}

void indicator_view_handler<platform::linux_>::apply_position(int v) {
    cached_position_ = v;
    // Only the selected/unselected class on each child needs to flip; the
    // simplest correct path is to rebuild — the cost is negligible at
    // typical basic_page counts.
    rebuild_dots();
}

void indicator_view_handler<platform::linux_>::apply_indicator_color(const brush_ref& b) {
    cached_color_ = b;
    reload_css();
}

void indicator_view_handler<platform::linux_>::apply_selected_indicator_color(const brush_ref& b) {
    cached_selected_ = b;
    reload_css();
}

void indicator_view_handler<platform::linux_>::map_count(basic_indicator_view& iv) {
    apply_count(iv.count.get());
    iv.count.changed.subscribe(count_slot_, count_cb_);
}
void indicator_view_handler<platform::linux_>::map_position(basic_indicator_view& iv) {
    apply_position(iv.position.get());
    iv.position.changed.subscribe(position_slot_, position_cb_);
}
void indicator_view_handler<platform::linux_>::map_indicator_color(basic_indicator_view& iv) {
    apply_indicator_color(iv.indicator_color.get());
    iv.indicator_color.changed.subscribe(color_slot_, color_cb_);
}
void indicator_view_handler<platform::linux_>::map_selected_indicator_color(basic_indicator_view& iv) {
    apply_selected_indicator_color(iv.selected_indicator_color.get());
    iv.selected_indicator_color.changed.subscribe(sel_color_slot_, sel_color_cb_);
}

} // namespace mpapp::internal
// ----- ADR-0013 self-registration --------------------------------------------

namespace {
GtkWidget* dispatch_indicator_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_indicator_view*>(v); w && w->has_handler()) {
        return static_cast<GtkWidget*>(w->handler().native());
    }
    return nullptr;
}
struct registrar {
    registrar() {
        ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_indicator_view);
    }
};
[[maybe_unused]] registrar _reg;
} // namespace

#endif // __linux__ && !__ANDROID__
