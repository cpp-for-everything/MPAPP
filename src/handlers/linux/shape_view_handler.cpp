// SPDX-License-Identifier: Apache-2.0
// GTK4 shape_view handler implementation. Renders via cairo inside
// the GtkDrawingArea's draw callback.

#include "mpapp/handlers/linux/shape_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <cctype>
#include <cstdlib>
#include <cstdint>

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp {

namespace {

struct rgba { double r, g, b, a; bool ok; };

rgba parse_color(const std::string& s) {
    rgba out{0, 0, 0, 0, false};
    if (s.empty() || s[0] != '#') return out;
    auto hex_val = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    auto byte = [&](size_t i) -> int {
        int hi = hex_val(s[i]);
        int lo = hex_val(s[i + 1]);
        if (hi < 0 || lo < 0) return -1;
        return (hi << 4) | lo;
    };
    if (s.size() == 7) {
        int r = byte(1), g = byte(3), b = byte(5);
        if (r < 0 || g < 0 || b < 0) return out;
        out = {r / 255.0, g / 255.0, b / 255.0, 1.0, true};
    } else if (s.size() == 9) {
        int a = byte(1), r = byte(3), g = byte(5), b = byte(7);
        if (a < 0 || r < 0 || g < 0 || b < 0) return out;
        out = {r / 255.0, g / 255.0, b / 255.0, a / 255.0, true};
    } else if (s.size() == 4) {
        int r = hex_val(s[1]), g = hex_val(s[2]), b = hex_val(s[3]);
        if (r < 0 || g < 0 || b < 0) return out;
        out = {(r * 0x11) / 255.0, (g * 0x11) / 255.0, (b * 0x11) / 255.0, 1.0, true};
    }
    return out;
}

bool parse_line(const std::string& data, double& x1, double& y1, double& x2, double& y2) {
    double v[4] = {0, 0, 0, 0};
    int n = 0;
    const char* p = data.c_str();
    while (*p && n < 4) {
        while (*p && !(std::isdigit(static_cast<unsigned char>(*p)) || *p == '-' || *p == '+' || *p == '.')) ++p;
        if (!*p) break;
        char* end = nullptr;
        v[n++] = std::strtod(p, &end);
        if (end == p) break;
        p = end;
    }
    if (n != 4) return false;
    x1 = v[0]; y1 = v[1]; x2 = v[2]; y2 = v[3];
    return true;
}

void draw_callback(GtkDrawingArea* /*area*/,
                   cairo_t* cr,
                   int width,
                   int height,
                   gpointer user_data) {
    auto* h = static_cast<shape_view_handler<platform::linux_>*>(user_data);
    if (h == nullptr || h->bound() == nullptr) return;
    shape_view* s = h->bound();
    const double opacity = s->opacity.get();
    const double t = s->stroke_thickness.get();
    rgba fill   = parse_color(s->fill.get());
    rgba stroke = parse_color(s->stroke.get());

    cairo_save(cr);
    cairo_set_line_width(cr, t);

    const double off = t * 0.5;
    const double w   = static_cast<double>(width)  - t;
    const double h_  = static_cast<double>(height) - t;

    auto paint_fill_then_stroke = [&]() {
        if (fill.ok) {
            cairo_set_source_rgba(cr, fill.r, fill.g, fill.b, fill.a * opacity);
            cairo_fill_preserve(cr);
        }
        if (stroke.ok) {
            cairo_set_source_rgba(cr, stroke.r, stroke.g, stroke.b, stroke.a * opacity);
            cairo_stroke(cr);
        } else {
            cairo_new_path(cr);
        }
    };

    switch (s->kind.get()) {
        case shape_kind::ellipse: {
            cairo_save(cr);
            cairo_translate(cr, off + w * 0.5, off + h_ * 0.5);
            cairo_scale(cr, w * 0.5, h_ * 0.5);
            cairo_arc(cr, 0, 0, 1, 0, 2 * 3.14159265358979);
            cairo_restore(cr);
            paint_fill_then_stroke();
            break;
        }
        case shape_kind::line: {
            double x1 = 0, y1 = 0, x2 = static_cast<double>(width), y2 = static_cast<double>(height);
            parse_line(s->data.get(), x1, y1, x2, y2);
            cairo_move_to(cr, x1, y1);
            cairo_line_to(cr, x2, y2);
            if (stroke.ok) {
                cairo_set_source_rgba(cr, stroke.r, stroke.g, stroke.b, stroke.a * opacity);
                cairo_stroke(cr);
            }
            break;
        }
        case shape_kind::rectangle:
        case shape_kind::polygon:  // v1: bounding rect
        case shape_kind::path:     // v1: bounding rect
        default: {
            cairo_rectangle(cr, off, off, w, h_);
            paint_fill_then_stroke();
            break;
        }
    }
    cairo_restore(cr);
}

} // namespace

shape_view_handler<platform::linux_>::shape_view_handler() {
    native_ = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(
        GTK_DRAWING_AREA(static_cast<GtkWidget*>(native_)),
        draw_callback,
        this,
        nullptr);
}

shape_view_handler<platform::linux_>::~shape_view_handler() = default;

void shape_view_handler<platform::linux_>::invalidate_() {
    if (native_ != nullptr) {
        gtk_widget_queue_draw(static_cast<GtkWidget*>(native_));
    }
}

void shape_view_handler<platform::linux_>::map_kind(shape_view& s) {
    bound_ = &s;
    invalidate_();
    s.kind.changed.subscribe(kind_slot_, kind_cb_);
}
void shape_view_handler<platform::linux_>::map_data(shape_view& s) {
    invalidate_();
    s.data.changed.subscribe(data_slot_, data_cb_);
}
void shape_view_handler<platform::linux_>::map_fill(shape_view& s) {
    invalidate_();
    s.fill.changed.subscribe(fill_slot_, fill_cb_);
}
void shape_view_handler<platform::linux_>::map_stroke(shape_view& s) {
    invalidate_();
    s.stroke.changed.subscribe(stroke_slot_, stroke_cb_);
}
void shape_view_handler<platform::linux_>::map_stroke_thickness(shape_view& s) {
    invalidate_();
    s.stroke_thickness.changed.subscribe(stroke_thick_slot_, stroke_thick_cb_);
}
void shape_view_handler<platform::linux_>::map_opacity(shape_view& s) {
    invalidate_();
    s.opacity.changed.subscribe(opacity_slot_, opacity_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_shape_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::shape_view*>(v); w && w->has_sv_handler()) {
        return GTK_WIDGET(w->sv_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_shape_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
