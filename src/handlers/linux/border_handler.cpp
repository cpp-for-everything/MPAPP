// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_border handler implementation.

#include "mpapp/handlers/linux/border_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

#include "mpapp/internal/basic_box_view.hpp"
#include "mpapp/internal/basic_button.hpp"
#include "mpapp/internal/basic_check_box.hpp"
#include "mpapp/editor.hpp"
#include "mpapp/internal/basic_entry.hpp"
#include "mpapp/handlers/linux/box_view_handler.hpp"
#include "mpapp/handlers/linux/button_handler.hpp"
#include "mpapp/handlers/linux/check_box_handler.hpp"
#include "mpapp/handlers/linux/editor_handler.hpp"
#include "mpapp/handlers/linux/entry_handler.hpp"
#include "mpapp/handlers/linux/label_handler.hpp"
#include "mpapp/handlers/linux/radio_button_handler.hpp"
#include "mpapp/handlers/linux/slider_handler.hpp"
#include "mpapp/handlers/linux/stack_layout_handler.hpp"
#include "mpapp/handlers/linux/stepper_handler.hpp"
#include "mpapp/handlers/linux/switch_handler.hpp"
#include "mpapp/internal/basic_label.hpp"
#include "mpapp/internal/basic_radio_button.hpp"
#include "mpapp/internal/basic_slider.hpp"
#include "mpapp/internal/basic_stack_layout.hpp"
#include "mpapp/internal/basic_stepper.hpp"
#include "mpapp/internal/basic_switch_.hpp"

namespace mpapp::internal {

namespace {

std::string next_border_class() {
    static std::atomic<unsigned> counter{0};
    return "mpapp-basic_border-" + std::to_string(counter.fetch_add(1));
}

// Same name set as Windows. Returns rgba doubles in 0..1.
struct rgba { double r=0, g=0, b=0, a=1; };

rgba parse_brush(const brush_ref& br) {
    rgba out{0, 0, 0, 1};
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
    if (name == "Black") return {0, 0, 0, 1};
    if (name == "White") return {1, 1, 1, 1};
    if (name == "Gray")  return {128/255.0, 128/255.0, 128/255.0, 1};
    if (name == "Teal")  return {0, 150/255.0, 165/255.0, 1};
    return out;
}

struct corner4 { double tl=0, tr=0, br=0, bl=0; };
corner4 parse_corners(const stroke_shape_desc& s) {
    corner4 out{};
    auto paren = s.descriptor.find('(');
    if (paren == std::string::npos) return out;
    auto close = s.descriptor.find(')', paren + 1);
    if (close == std::string::npos) return out;
    std::string args = s.descriptor.substr(paren + 1, close - paren - 1);
    double values[4]{};
    int n = 0;
    std::string cur;
    for (char c : args) {
        if (c == ',') { if (n < 4) values[n++] = std::atof(cur.c_str()); cur.clear(); }
        else if (c != ' ') cur.push_back(c);
    }
    if (!cur.empty() && n < 4) values[n++] = std::atof(cur.c_str());
    if (n == 1)      out = {values[0], values[0], values[0], values[0]};
    else if (n == 4) out = {values[0], values[1], values[2], values[3]};
    return out;
}

GtkWidget* native_widget_of(view* v) {
    // ADR-0013: registry dispatch only — each widget self-registers.
    return detail::linux_dispatch::dispatch(v);
}

int to255(double v) {
    if (!(v == v)) return 0;
    if (v <= 0.0) return 0;
    if (v >= 1.0) return 255;
    return static_cast<int>(v * 255.0 + 0.5);
}

} // namespace

border_handler<platform::linux_>::border_handler() {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    native_ = box;

    class_name_ = next_border_class();
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

border_handler<platform::linux_>::~border_handler() {
    if (provider_ != nullptr) {
        if (GdkDisplay* display = gdk_display_get_default(); display != nullptr) {
            gtk_style_context_remove_provider_for_display(
                display, GTK_STYLE_PROVIDER(static_cast<GtkCssProvider*>(provider_)));
        }
        g_object_unref(provider_);
        provider_ = nullptr;
    }
}

void border_handler<platform::linux_>::reload_css() {
    if (provider_ == nullptr) return;
    rgba c = parse_brush(cached_stroke_);
    corner4 r = parse_corners(cached_stroke_shape_);
    char buf[512];
    std::snprintf(
        buf, sizeof(buf),
        ".%s { basic_border: %.1fpx solid rgba(%d,%d,%d,%.3f); "
        "basic_border-radius: %.1fpx %.1fpx %.1fpx %.1fpx; "
        "padding: %.1fpx %.1fpx %.1fpx %.1fpx; }",
        class_name_.c_str(),
        cached_stroke_thickness_,
        to255(c.r), to255(c.g), to255(c.b), c.a,
        r.tl, r.tr, r.br, r.bl,
        cached_padding_.top, cached_padding_.right,
        cached_padding_.bottom, cached_padding_.left);
    gtk_css_provider_load_from_data(
        static_cast<GtkCssProvider*>(provider_), buf, -1);
}

void border_handler<platform::linux_>::apply_content(const std::shared_ptr<view>& v) {
    GtkBox* box = GTK_BOX(static_cast<GtkWidget*>(native_));
    if (current_child_ != nullptr) {
        gtk_box_remove(box, GTK_WIDGET(current_child_));
        current_child_ = nullptr;
    }
    GtkWidget* child = v ? native_widget_of(v.get()) : nullptr;
    if (child != nullptr) {
        gtk_box_append(box, child);
        current_child_ = child;
    }
}

void border_handler<platform::linux_>::apply_padding(const thickness& t)         { cached_padding_           = t; reload_css(); }
void border_handler<platform::linux_>::apply_stroke(const brush_ref& b)          { cached_stroke_            = b; reload_css(); }
void border_handler<platform::linux_>::apply_stroke_thickness(double t)          { cached_stroke_thickness_  = t; reload_css(); }
void border_handler<platform::linux_>::apply_stroke_shape(const stroke_shape_desc& s) { cached_stroke_shape_ = s; reload_css(); }

void border_handler<platform::linux_>::map_content(basic_border& b)          { apply_content(b.content.get()); b.content.changed.subscribe(content_slot_, content_cb_); }
void border_handler<platform::linux_>::map_padding(basic_border& b)          { apply_padding(b.padding.get()); b.padding.changed.subscribe(padding_slot_, padding_cb_); }
void border_handler<platform::linux_>::map_stroke(basic_border& b)           { apply_stroke(b.stroke.get()); b.stroke.changed.subscribe(stroke_slot_, stroke_cb_); }
void border_handler<platform::linux_>::map_stroke_thickness(basic_border& b) { apply_stroke_thickness(b.stroke_thickness.get()); b.stroke_thickness.changed.subscribe(stroke_thick_slot_, stroke_thick_cb_); }
void border_handler<platform::linux_>::map_stroke_shape(basic_border& b)     { apply_stroke_shape(b.stroke_shape.get()); b.stroke_shape.changed.subscribe(stroke_shape_slot_, stroke_shape_cb_); }

void border_handler<platform::linux_>::bind_content(basic_border& b, view& child) {
    b.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
namespace {

GtkWidget* dispatch_border(::mpapp::view* v) {
    if (auto* b = dynamic_cast<::mpapp::internal::basic_border*>(v); b && b->has_handler()) {
        return GTK_WIDGET(b->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_border); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
