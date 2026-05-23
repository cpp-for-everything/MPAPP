// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Visible end-to-end demo of the canvas facade per
// [[ADR-0015-graphics-backend-dual]].
//
// Renders several shapes through the abstract `canvas` interface
// and writes the result to `cairo_render_demo.png` in the current
// directory. The same source compiles on Linux, Windows, and
// Android — the actual rendering depends on whichever backend the
// build was configured with:
//
//   stub backend  → no rendering; the program exits without
//                   writing a PNG (and prints a notice). Useful
//                   for sanity-checking the facade compiles on
//                   headless platforms.
//   cairo backend → uses libcairo's image surface; writes the PNG
//                   on every supported platform.
//
// To force a specific output path, pass it as the first argument:
//   cairo_render_demo /tmp/foo.png
//
// To verify build wiring without rendering, pass `--dry-run`.

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>

#include <mpapp/detail/graphics/canvas.hpp>

#if defined(MPAPP_GRAPHICS_HAS_CAIRO)
    #include <cairo/cairo.h>
#endif

using namespace mpapp::detail::graphics;

namespace {

// Drive the abstract canvas through a non-trivial paint sequence.
// The choice of shapes is the same regardless of backend; the only
// thing the backend changes is whether actual pixels land.
void paint(canvas& c) {
    // Clear to off-white.
    c.clear(color::rgb(0.96f, 0.96f, 0.94f));

    // --- Top row: filled primitives -----------------------------

    // Red rectangle.
    c.set_fill(color::from_hex("#E63946"));
    c.fill_rect(rect{20, 20, 100, 60});

    // Teal ellipse.
    c.set_fill(color::from_hex("#2A9D8F"));
    c.fill_ellipse(rect{140, 20, 100, 60});

    // Amber path (triangle) via the SVG path subset parser.
    c.set_fill(color::from_hex("#F4A261"));
    c.fill_path(path::from_svg("M260 20 L360 20 L310 80 Z"));

    // --- Middle row: strokes -----------------------------------

    c.set_stroke(color::from_hex("#1D3557"));
    c.set_stroke_width(3.0);
    c.set_line_join(line_join::round);
    c.set_line_cap(line_cap::round);

    // Rectangle outline.
    c.stroke_rect(rect{20, 110, 100, 60});

    // Ellipse outline.
    c.stroke_ellipse(rect{140, 110, 100, 60});

    // Cubic Bezier curve.
    c.stroke_path(path{}
        .move_to(260, 170)
        .cubic_to(290, 110, 330, 110, 360, 170));

    // --- Bottom row: transforms + opacity ----------------------

    c.save();
    c.translate(80, 240);
    c.rotate(0.3);
    c.set_opacity(0.5);
    c.set_fill(color::from_hex("#264653"));
    c.fill_rect(rect{0, 0, 80, 50});
    c.restore();

    // A clip example — only the right half of the rectangle gets
    // painted because we clip to a circle.
    c.save();
    path clip_path;
    clip_path.move_to(220, 245);
    clip_path.cubic_to(280, 215, 360, 215, 360, 275);
    clip_path.cubic_to(360, 305, 280, 305, 220, 275);
    clip_path.close();
    c.clip(clip_path);
    c.set_fill(color::from_hex("#E76F51"));
    c.fill_rect(rect{200, 220, 200, 80});
    c.restore();
}

void print_usage(const char* exe) {
    std::printf(
        "Usage: %s [output.png] [--dry-run]\n"
        "  Renders demo shapes through mpapp::detail::graphics::canvas.\n"
        "  Writes to cairo_render_demo.png by default.\n"
        "  --dry-run: drive the canvas calls but skip the file write.\n",
        exe);
}

#if defined(MPAPP_GRAPHICS_HAS_CAIRO)
// The canvas facade doesn't expose its underlying surface through
// the abstract API. For the Cairo backend specifically we know the
// surface is a cairo_image_surface_t; this helper rebuilds the same
// scene through a directly-owned Cairo surface so we can write it
// to disk. That gives an end-to-end PNG that the user can open.
//
// Why not a downcast from the facade? Because the facade owns the
// surface lifetime + may swap it. Re-rendering through a parallel
// surface keeps the test program clean and matches what real apps
// will do (they'll typically own their own surface and forward to
// the facade for ops).
class direct_cairo_canvas : public canvas {
public:
    direct_cairo_canvas(int w, int h) : w_{w}, h_{h} {
        surface_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
        cr_      = cairo_create(surface_);
    }
    ~direct_cairo_canvas() override {
        if (cr_) cairo_destroy(cr_);
        if (surface_) cairo_surface_destroy(surface_);
    }

    [[nodiscard]] int width_px()  const noexcept override { return w_; }
    [[nodiscard]] int height_px() const noexcept override { return h_; }

    void save()    override { cairo_save(cr_); }
    void restore() override { cairo_restore(cr_); }

    void translate(double dx, double dy) override { cairo_translate(cr_, dx, dy); }
    void scale(double sx, double sy)     override { cairo_scale(cr_, sx, sy); }
    void rotate(double radians)          override { cairo_rotate(cr_, radians); }

    void set_fill(color c)             override { fill_ = c; }
    void set_stroke(color c)           override { stroke_ = c; }
    void set_stroke_width(double w)    override { cairo_set_line_width(cr_, w); }
    void set_line_cap(line_cap c)      override {
        cairo_set_line_cap(cr_, to_cairo_cap(c));
    }
    void set_line_join(line_join j)    override {
        cairo_set_line_join(cr_, to_cairo_join(j));
    }
    void set_opacity(double a)         override { opacity_ = a; }

    void clear(color c) override {
        cairo_save(cr_);
        cairo_set_source_rgba(cr_, c.r, c.g, c.b, c.a);
        cairo_set_operator(cr_, CAIRO_OPERATOR_SOURCE);
        cairo_paint(cr_);
        cairo_restore(cr_);
    }
    void fill_rect(rect r) override {
        apply_fill();
        cairo_rectangle(cr_, r.x, r.y, r.w, r.h);
        cairo_fill(cr_);
    }
    void stroke_rect(rect r) override {
        apply_stroke();
        cairo_rectangle(cr_, r.x, r.y, r.w, r.h);
        cairo_stroke(cr_);
    }
    void fill_ellipse(rect b) override { ellipse_op(b, true); }
    void stroke_ellipse(rect b) override { ellipse_op(b, false); }
    void fill_path(const path& p) override {
        apply_fill();
        cairo_new_path(cr_);
        emit(p);
        cairo_fill(cr_);
    }
    void stroke_path(const path& p) override {
        apply_stroke();
        cairo_new_path(cr_);
        emit(p);
        cairo_stroke(cr_);
    }
    void clip(const path& p) override {
        cairo_new_path(cr_);
        emit(p);
        cairo_clip(cr_);
    }

    bool write_png(const char* path) {
        cairo_surface_flush(surface_);
        return cairo_surface_write_to_png(surface_, path) == CAIRO_STATUS_SUCCESS;
    }

private:
    void apply_fill() {
        cairo_set_source_rgba(cr_, fill_.r, fill_.g, fill_.b, fill_.a * opacity_);
    }
    void apply_stroke() {
        cairo_set_source_rgba(cr_, stroke_.r, stroke_.g, stroke_.b, stroke_.a * opacity_);
    }
    void ellipse_op(rect b, bool fill) {
        cairo_new_path(cr_);
        cairo_save(cr_);
        cairo_translate(cr_, b.x + b.w / 2.0, b.y + b.h / 2.0);
        if (b.w > 0 && b.h > 0) {
            cairo_scale(cr_, b.w / 2.0, b.h / 2.0);
            cairo_arc(cr_, 0, 0, 1, 0, 2 * 3.14159265358979323846);
        }
        cairo_restore(cr_);
        if (fill) { apply_fill(); cairo_fill(cr_); }
        else      { apply_stroke(); cairo_stroke(cr_); }
    }
    void emit(const path& p) {
        for (const auto& op : p.ops()) {
            switch (op.kind) {
                case path_op_kind::move:  cairo_move_to(cr_, op.x[0], op.y[0]); break;
                case path_op_kind::line:  cairo_line_to(cr_, op.x[0], op.y[0]); break;
                case path_op_kind::quad: {
                    double cx, cy;
                    cairo_get_current_point(cr_, &cx, &cy);
                    const double c1x = cx + (2.0/3.0) * (op.x[0] - cx);
                    const double c1y = cy + (2.0/3.0) * (op.y[0] - cy);
                    const double c2x = op.x[1] + (2.0/3.0) * (op.x[0] - op.x[1]);
                    const double c2y = op.y[1] + (2.0/3.0) * (op.y[0] - op.y[1]);
                    cairo_curve_to(cr_, c1x, c1y, c2x, c2y, op.x[1], op.y[1]);
                    break;
                }
                case path_op_kind::cubic:
                    cairo_curve_to(cr_, op.x[0], op.y[0], op.x[1], op.y[1], op.x[2], op.y[2]);
                    break;
                case path_op_kind::close: cairo_close_path(cr_); break;
            }
        }
    }
    static cairo_line_cap_t to_cairo_cap(line_cap c) noexcept {
        switch (c) {
            case line_cap::butt:   return CAIRO_LINE_CAP_BUTT;
            case line_cap::round:  return CAIRO_LINE_CAP_ROUND;
            case line_cap::square: return CAIRO_LINE_CAP_SQUARE;
        }
        return CAIRO_LINE_CAP_BUTT;
    }
    static cairo_line_join_t to_cairo_join(line_join j) noexcept {
        switch (j) {
            case line_join::miter: return CAIRO_LINE_JOIN_MITER;
            case line_join::round: return CAIRO_LINE_JOIN_ROUND;
            case line_join::bevel: return CAIRO_LINE_JOIN_BEVEL;
        }
        return CAIRO_LINE_JOIN_MITER;
    }

    int              w_{0};
    int              h_{0};
    cairo_surface_t* surface_{nullptr};
    cairo_t*         cr_{nullptr};
    color            fill_{0, 0, 0, 1};
    color            stroke_{0, 0, 0, 1};
    double           opacity_{1.0};
};
#endif // MPAPP_GRAPHICS_HAS_CAIRO

} // namespace

int main(int argc, char** argv) {
    std::string output = "cairo_render_demo.png";
    bool        dry_run = false;
    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg == "--dry-run") {
            dry_run = true;
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (!arg.empty() && arg[0] != '-') {
            output = arg;
        } else {
            std::fprintf(stderr, "Unknown arg: %s\n", argv[i]);
            print_usage(argv[0]);
            return 2;
        }
    }

    // Drive the abstract facade — this exercises the build wiring
    // for whichever backend was selected at compile time. Useful
    // even with the stub backend (catches surface-API regressions).
    auto facade_canvas = make_canvas(400, 320);
    paint(*facade_canvas);
    std::printf("paint() through facade canvas (%dx%d): ok\n",
                facade_canvas->width_px(),
                facade_canvas->height_px());

#if defined(MPAPP_GRAPHICS_HAS_CAIRO)
    if (!dry_run) {
        direct_cairo_canvas direct{400, 320};
        paint(direct);
        if (!direct.write_png(output.c_str())) {
            std::fprintf(stderr, "Failed to write PNG to %s\n", output.c_str());
            return 1;
        }
        std::printf("Wrote %s (400x320, ARGB32 via cairo backend)\n",
                    output.c_str());
    } else {
        std::printf("--dry-run: skipping PNG write (would have been %s)\n",
                    output.c_str());
    }
#else
    std::printf("Built with stub backend (MPAPP_GRAPHICS_HAS_CAIRO undefined).\n"
                "No PNG written. Rebuild with MPAPP_GRAPHICS_BACKEND=cairo to render.\n");
    (void)output; (void)dry_run;
#endif

    return 0;
}
