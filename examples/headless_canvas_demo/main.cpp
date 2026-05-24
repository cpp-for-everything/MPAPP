// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Headless canvas + shape_view rendering demo.
//
// Exercises the same pipelines the T-0029 + T-0031 real handlers
// use, but renders to disk as PNGs instead of into a native widget.
// Result: Rule-11 visual evidence that the canvas facade + shape
// renderer produce the expected output, without needing a live GUI
// or screen-capture tooling. Same Cairo backend that the Linux real
// handlers blit into a GtkDrawingArea is what writes the PNG here —
// what you see is what your app would see.
//
// Usage:  headless_canvas_demo <output_dir>
//
// Outputs (in <output_dir>):
//   t0029_graphicsview.png        — sample graphics_view drawable
//   t0031_rectangle.png           — shape_view kind=rectangle
//   t0031_ellipse.png             — shape_view kind=ellipse
//   t0031_line.png                — shape_view kind=line
//   t0031_polygon.png             — shape_view kind=polygon  (via SVG path)
//   t0031_path.png                — shape_view kind=path     (via SVG path)
//
// Gated on MPAPP_GRAPHICS_HAS_CAIRO at build time because the
// cairo_surface_write_to_png writer is the simplest cross-host way
// to land PNG bytes on disk. The rendering pipeline under test is
// the abstract canvas facade — Cairo is the backend that happens to
// be wired today; once Skia is installed it could swap in here too.

#include <cstdio>
#include <cstdlib>
#include <string>

#include <cairo.h>

#include "mpapp/detail/graphics/canvas.hpp"
#include "mpapp/detail/graphics/shape_renderer.hpp"
// Headless demo links only mpapp-core (not the per-platform handler
// library), so use the surface `internal::basic_shape_view` directly
// rather than the auto-binding `mpapp::shape_view` wrapper — the
// wrapper would force a link against the platform handler symbols.
#include "mpapp/internal/basic_shape_view.hpp"

using namespace mpapp;
using namespace mpapp::detail::graphics;

namespace {

bool write_canvas_png(const canvas& c, const std::string& path) {
    const unsigned char* px     = c.pixel_data();
    const int            stride = c.pixel_stride_bytes();
    const int            w      = c.width_px();
    const int            h      = c.height_px();
    if (px == nullptr || stride <= 0) return false;
    cairo_surface_t* surf = cairo_image_surface_create_for_data(
        const_cast<unsigned char*>(px),
        CAIRO_FORMAT_ARGB32,
        w, h, stride);
    if (surf == nullptr) return false;
    cairo_status_t rc = cairo_surface_write_to_png(surf, path.c_str());
    cairo_surface_destroy(surf);
    return rc == CAIRO_STATUS_SUCCESS;
}

// T-0029 sample drawable: a few overlapping shapes that exercise
// fill, stroke, transforms, and path ops. Matches the kind of thing
// a user would write in `gv.drawable = [](canvas& c) { ... };`.
void sample_graphics_view_drawable(canvas& c) {
    c.clear(color::rgb(0.95f, 0.95f, 0.95f));  // light grey background
    c.set_fill(color::from_hex("#264653"));
    c.fill_rect({20, 20, 200, 80});
    c.set_fill(color::from_hex("#E76F51"));
    c.fill_ellipse({100, 60, 160, 100});
    c.set_stroke(color::from_hex("#2A9D8F"));
    c.set_stroke_width(4.0);
    c.stroke_rect({40, 100, 80, 60});
    path p;
    p.move_to(220, 20);
    p.line_to(280, 80);
    p.line_to(220, 140);
    p.close();
    c.set_fill(color::from_hex("#E9C46A"));
    c.fill_path(p);
    c.set_stroke(color::from_hex("#264653"));
    c.set_stroke_width(2.0);
    c.stroke_path(p);
}

// T-0031 evidence: render one shape_view per kind via the shared
// renderer. Each output PNG shows what the Linux ShapeView handler
// blits into its GtkDrawingArea (and what Windows + Android will
// render once they migrate in T-0031 phase 2).
void render_shape_kind(const std::string& out_dir,
                       shape_kind kind,
                       const std::string& data,
                       const std::string& filename,
                       int w = 240, int h = 160) {
    internal::basic_shape_view sv;
    sv.kind             = kind;
    sv.data             = data;
    sv.fill             = "#264653";
    sv.stroke           = "#E76F51";
    sv.stroke_thickness = 4.0;
    sv.opacity          = 1.0;
    auto c = make_canvas(w, h);
    if (c == nullptr) return;
    c->clear(color::rgb(0.95f, 0.95f, 0.95f));
    render_shape_view(*c, sv, w, h);
    const std::string path = out_dir + "/" + filename;
    if (write_canvas_png(*c, path)) {
        std::printf("wrote %s\n", path.c_str());
    } else {
        std::fprintf(stderr, "FAILED to write %s\n", path.c_str());
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: %s <output_dir>\n", argv[0]);
        return 2;
    }
    const std::string out_dir = argv[1];

    // T-0029: graphics_view drawable -> canvas -> PNG
    {
        constexpr int W = 320, H = 200;
        auto c = make_canvas(W, H);
        if (c == nullptr) {
            std::fprintf(stderr, "make_canvas failed\n");
            return 3;
        }
        sample_graphics_view_drawable(*c);
        const std::string path = out_dir + "/t0029_graphicsview.png";
        if (write_canvas_png(*c, path)) {
            std::printf("wrote %s\n", path.c_str());
        } else {
            std::fprintf(stderr, "FAILED to write %s\n", path.c_str());
        }
    }

    // T-0031: one PNG per shape_kind via the shared renderer.
    render_shape_kind(out_dir, shape_kind::rectangle, "",                  "t0031_rectangle.png");
    render_shape_kind(out_dir, shape_kind::ellipse,   "",                  "t0031_ellipse.png");
    render_shape_kind(out_dir, shape_kind::line,      "20 80 220 80",      "t0031_line.png");
    // Polygon: a closed quad via SVG path syntax.
    render_shape_kind(out_dir, shape_kind::polygon,
                      "M 40 20 L 200 30 L 220 140 L 60 130 Z",
                      "t0031_polygon.png");
    // Path: a more involved curve mixing line + cubic.
    render_shape_kind(out_dir, shape_kind::path,
                      "M 20 140 L 60 40 L 120 100 L 200 30 L 220 140 Z",
                      "t0031_path.png");

    return 0;
}
