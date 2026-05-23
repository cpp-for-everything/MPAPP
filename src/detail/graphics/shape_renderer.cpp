// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Shared shape_view rendering — see header.

#include "mpapp/detail/graphics/shape_renderer.hpp"

#include <cctype>
#include <cstdlib>
#include <string>
#include <string_view>

#include "mpapp/detail/graphics/canvas.hpp"
#include "mpapp/shape_view.hpp"

namespace mpapp::detail::graphics {

namespace {

// Parse exactly 4 floats from a SVG-like data string (e.g.
// "M0 0 L100 100"). Whitespace, letters, and commas are all treated
// as separators. Returns true iff 4 numbers were extracted.
bool parse_line(std::string_view data, double& x1, double& y1, double& x2, double& y2) {
    double v[4] = {0, 0, 0, 0};
    int n = 0;
    const char* p   = data.data();
    const char* end = p + data.size();
    while (p < end && n < 4) {
        while (p < end &&
               !(std::isdigit(static_cast<unsigned char>(*p)) || *p == '-' || *p == '+' || *p == '.')) {
            ++p;
        }
        if (p >= end) break;
        char* tail = nullptr;
        v[n++] = std::strtod(p, &tail);
        if (tail == p) break;
        p = tail;
    }
    if (n != 4) return false;
    x1 = v[0]; y1 = v[1]; x2 = v[2]; y2 = v[3];
    return true;
}

} // namespace

void render_shape_view(canvas& c, const shape_view& sv, int w, int h) {
    if (w <= 0 || h <= 0) return;

    const double thickness   = sv.stroke_thickness.get();
    const double opacity     = sv.opacity.get();
    const std::string& fillS   = sv.fill.get();
    const std::string& strokeS = sv.stroke.get();
    const bool   has_fill   = !fillS.empty();
    const bool   has_stroke = !strokeS.empty();
    const color  fill_color   = has_fill   ? color::from_hex(fillS)   : color{};
    const color  stroke_color = has_stroke ? color::from_hex(strokeS) : color{};

    // Inset the bounding box by half the stroke thickness so the
    // stroke draws fully inside the (w, h) region — matches what the
    // existing Cairo Linux handler did before this migration.
    const double off  = thickness * 0.5;
    const double dw   = static_cast<double>(w) - thickness;
    const double dh   = static_cast<double>(h) - thickness;

    c.save();
    c.set_opacity(opacity);
    c.set_stroke_width(thickness);
    if (has_fill)   c.set_fill(fill_color);
    if (has_stroke) c.set_stroke(stroke_color);

    auto paint_rect = [&](rect r) {
        if (has_fill)   c.fill_rect(r);
        if (has_stroke) c.stroke_rect(r);
    };
    auto paint_ellipse = [&](rect r) {
        if (has_fill)   c.fill_ellipse(r);
        if (has_stroke) c.stroke_ellipse(r);
    };
    auto paint_path = [&](const path& p) {
        if (has_fill)   c.fill_path(p);
        if (has_stroke) c.stroke_path(p);
    };

    switch (sv.kind.get()) {
        case shape_kind::ellipse:
            paint_ellipse(rect{off, off, dw, dh});
            break;
        case shape_kind::line: {
            double x1 = 0, y1 = 0;
            double x2 = static_cast<double>(w), y2 = static_cast<double>(h);
            parse_line(sv.data.get(), x1, y1, x2, y2);
            path p;
            p.move_to(x1, y1);
            p.line_to(x2, y2);
            if (has_stroke) c.stroke_path(p);
            // Lines never fill — even when fill is set, MAUI's
            // Line.Fill is a no-op. Match that behavior.
            break;
        }
        case shape_kind::polygon:
        case shape_kind::path: {
            // SVG-path parser is on the facade's `path::from_svg`.
            // Falls back to the bounding rectangle when the data
            // string doesn't parse, matching the legacy Linux v1
            // behavior.
            path p = path::from_svg(sv.data.get());
            if (!p.empty()) {
                paint_path(p);
            } else {
                paint_rect(rect{off, off, dw, dh});
            }
            break;
        }
        case shape_kind::rectangle:
        default:
            paint_rect(rect{off, off, dw, dh});
            break;
    }
    c.restore();
}

} // namespace mpapp::detail::graphics
