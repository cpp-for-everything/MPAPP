// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Cairo backend for the 2D graphics facade per
// [[ADR-0015-graphics-backend-dual]].
//
// Implements `mpapp::detail::graphics::canvas` against libcairo. The
// in-memory `cairo_image_surface_t` is the default render target —
// future work may add a constructor that wraps a foreign surface
// (e.g. a `GdkSurface` from GTK4) so the framework can paint directly
// into a window without copying.
//
// Build wiring (root CMakeLists.txt):
//   - MPAPP_GRAPHICS_BACKEND=cairo  → compile this TU + link Cairo.
//   - Cairo not found at configure time → fall back to stub_backend.cpp
//     with a CMake warning.
//
// License posture (RFC-0001 §Linux): Cairo is LGPL-2.1; we dynamic-
// link against the system libcairo and publish the rebuild path. No
// static linking; no source vendoring.

#include <mpapp/detail/graphics/canvas.hpp>

#include <cairo/cairo.h>

#include <cstdint>
#include <memory>

namespace mpapp::detail::graphics {

namespace {

cairo_line_cap_t to_cairo(line_cap c) noexcept {
    switch (c) {
        case line_cap::butt:   return CAIRO_LINE_CAP_BUTT;
        case line_cap::round:  return CAIRO_LINE_CAP_ROUND;
        case line_cap::square: return CAIRO_LINE_CAP_SQUARE;
    }
    return CAIRO_LINE_CAP_BUTT;
}

cairo_line_join_t to_cairo(line_join j) noexcept {
    switch (j) {
        case line_join::miter: return CAIRO_LINE_JOIN_MITER;
        case line_join::round: return CAIRO_LINE_JOIN_ROUND;
        case line_join::bevel: return CAIRO_LINE_JOIN_BEVEL;
    }
    return CAIRO_LINE_JOIN_MITER;
}

// Walk an mpapp `path` and emit Cairo path ops on `cr`. Caller is
// responsible for `cairo_new_path` before + `cairo_fill` / `_stroke`
// / `_clip` after.
void emit_path(cairo_t* cr, const path& p) {
    for (const auto& op : p.ops()) {
        switch (op.kind) {
            case path_op_kind::move:
                cairo_move_to(cr, op.x[0], op.y[0]);
                break;
            case path_op_kind::line:
                cairo_line_to(cr, op.x[0], op.y[0]);
                break;
            case path_op_kind::quad: {
                // Cairo only does cubic Bezier. Convert quad → cubic
                // by lifting the single control point: c1 = start +
                // 2/3 * (ctrl - start), c2 = end + 2/3 * (ctrl - end).
                double cur_x = 0.0;
                double cur_y = 0.0;
                cairo_get_current_point(cr, &cur_x, &cur_y);
                const double cx = op.x[0];
                const double cy = op.y[0];
                const double ex = op.x[1];
                const double ey = op.y[1];
                const double c1x = cur_x + (2.0 / 3.0) * (cx - cur_x);
                const double c1y = cur_y + (2.0 / 3.0) * (cy - cur_y);
                const double c2x = ex   + (2.0 / 3.0) * (cx - ex);
                const double c2y = ey   + (2.0 / 3.0) * (cy - ey);
                cairo_curve_to(cr, c1x, c1y, c2x, c2y, ex, ey);
                break;
            }
            case path_op_kind::cubic:
                cairo_curve_to(cr,
                               op.x[0], op.y[0],
                               op.x[1], op.y[1],
                               op.x[2], op.y[2]);
                break;
            case path_op_kind::close:
                cairo_close_path(cr);
                break;
        }
    }
}

} // namespace

class cairo_canvas final : public canvas {
public:
    cairo_canvas(int w, int h) : w_{w}, h_{h} {
        surface_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
        cr_      = cairo_create(surface_);
    }
    ~cairo_canvas() override {
        if (cr_      != nullptr) cairo_destroy(cr_);
        if (surface_ != nullptr) cairo_surface_destroy(surface_);
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
    void set_line_cap(line_cap c)      override { cairo_set_line_cap(cr_, to_cairo(c)); }
    void set_line_join(line_join j)    override { cairo_set_line_join(cr_, to_cairo(j)); }
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

    void fill_ellipse(rect b) override {
        apply_fill();
        ellipse_path(b);
        cairo_fill(cr_);
    }
    void stroke_ellipse(rect b) override {
        apply_stroke();
        ellipse_path(b);
        cairo_stroke(cr_);
    }

    void fill_path(const path& p) override {
        apply_fill();
        cairo_new_path(cr_);
        emit_path(cr_, p);
        cairo_fill(cr_);
    }
    void stroke_path(const path& p) override {
        apply_stroke();
        cairo_new_path(cr_);
        emit_path(cr_, p);
        cairo_stroke(cr_);
    }

    void clip(const path& p) override {
        cairo_new_path(cr_);
        emit_path(cr_, p);
        cairo_clip(cr_);
    }

    // Backend-specific extension: lets test code read back pixels to
    // verify rendering. Not part of the abstract `canvas` interface —
    // tests must downcast (or use the helper accessor below).
    [[nodiscard]] cairo_surface_t* native_surface() noexcept { return surface_; }

private:
    void apply_fill() {
        cairo_set_source_rgba(cr_, fill_.r, fill_.g, fill_.b, fill_.a * opacity_);
    }
    void apply_stroke() {
        cairo_set_source_rgba(cr_, stroke_.r, stroke_.g, stroke_.b, stroke_.a * opacity_);
    }

    // Cairo has no native ellipse op — center + scale + arc is the
    // idiomatic trick. We save/restore the matrix so the user's
    // transform isn't permanently clobbered.
    void ellipse_path(rect b) {
        cairo_new_path(cr_);
        cairo_save(cr_);
        cairo_translate(cr_, b.x + b.w / 2.0, b.y + b.h / 2.0);
        if (b.w > 0.0 && b.h > 0.0) {
            cairo_scale(cr_, b.w / 2.0, b.h / 2.0);
            cairo_arc(cr_, 0.0, 0.0, 1.0, 0.0, 2.0 * 3.14159265358979323846);
        }
        cairo_restore(cr_);
    }

    int              w_{0};
    int              h_{0};
    cairo_surface_t* surface_{nullptr};
    cairo_t*         cr_{nullptr};
    color            fill_{0.0f, 0.0f, 0.0f, 1.0f};
    color            stroke_{0.0f, 0.0f, 0.0f, 1.0f};
    double           opacity_{1.0};
};

std::unique_ptr<canvas> make_canvas(int width_px, int height_px) {
    return std::make_unique<cairo_canvas>(width_px, height_px);
}

} // namespace mpapp::detail::graphics
