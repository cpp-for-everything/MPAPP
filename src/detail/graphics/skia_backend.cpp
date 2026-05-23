// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Skia implementation of the ADR-0015 graphics facade.
//
// Selected at build time via `-DMPAPP_GRAPHICS_BACKEND=skia`. The CMake
// detection step (`find_package(unofficial-skia)`) links the vcpkg
// imported target `unofficial::skia::skia`, which carries Skia's
// include directories + transitive deps (libpng, freetype, harfbuzz,
// expat, ...). If Skia is selected but not installed, CMake falls back
// to the stub backend and emits a clear install hint — so this TU
// only compiles when Skia headers + libs are present.
//
// Implementation notes:
//   * The backing surface is an `SkBitmap` of type `kN32_SkColorType`,
//     which is the platform-native 32-bit format. On the little-endian
//     platforms MPAPP targets this is BGRA premultiplied — matching
//     Cairo's CAIRO_FORMAT_ARGB32 byte ordering and the format
//     documented on the abstract canvas API for pixel_data() readback.
//   * SkPaint is rebuilt fresh on each draw op so the stored fill /
//     stroke / cap / join / opacity state always reflects what the
//     caller asked for, without us tracking dirty bits.
//   * SkCanvas::rotate takes degrees, but the facade exposes radians
//     (matching Cairo). The wrapper converts at the boundary.

#include "mpapp/detail/graphics/canvas.hpp"

#if defined(MPAPP_GRAPHICS_HAS_SKIA)

// Skia's idiomatic include style: header paths start with `include/`
// because that's the top of the Skia source tree, and vcpkg preserves
// that layout (adding `<vcpkg>/installed/<triplet>/include/skia/` to
// the include path). Looks unusual but is the upstream-Skia
// convention.
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"

#include <cmath>
#include <memory>

namespace mpapp::detail::graphics {

namespace {

constexpr double kPi = 3.14159265358979323846;

SkPaint::Cap to_skia_cap(line_cap c) noexcept {
    switch (c) {
        case line_cap::butt:   return SkPaint::Cap::kButt_Cap;
        case line_cap::round:  return SkPaint::Cap::kRound_Cap;
        case line_cap::square: return SkPaint::Cap::kSquare_Cap;
    }
    return SkPaint::Cap::kButt_Cap;
}

SkPaint::Join to_skia_join(line_join j) noexcept {
    switch (j) {
        case line_join::miter: return SkPaint::Join::kMiter_Join;
        case line_join::round: return SkPaint::Join::kRound_Join;
        case line_join::bevel: return SkPaint::Join::kBevel_Join;
    }
    return SkPaint::Join::kMiter_Join;
}

// Build an SkPath from the facade's op-list `path`. The op kinds
// were chosen to mirror SVG / Skia primitives so the translation is
// 1:1 — no flattening or polyline approximation needed.
SkPath to_skia_path(const path& p) {
    SkPath sp;
    for (const auto& op : p.ops()) {
        switch (op.kind) {
            case path_op_kind::move:
                sp.moveTo(SkDoubleToScalar(op.x[0]),
                          SkDoubleToScalar(op.y[0]));
                break;
            case path_op_kind::line:
                sp.lineTo(SkDoubleToScalar(op.x[0]),
                          SkDoubleToScalar(op.y[0]));
                break;
            case path_op_kind::quad:
                sp.quadTo(SkDoubleToScalar(op.x[0]),
                          SkDoubleToScalar(op.y[0]),
                          SkDoubleToScalar(op.x[1]),
                          SkDoubleToScalar(op.y[1]));
                break;
            case path_op_kind::cubic:
                sp.cubicTo(SkDoubleToScalar(op.x[0]),
                           SkDoubleToScalar(op.y[0]),
                           SkDoubleToScalar(op.x[1]),
                           SkDoubleToScalar(op.y[1]),
                           SkDoubleToScalar(op.x[2]),
                           SkDoubleToScalar(op.y[2]));
                break;
            case path_op_kind::close:
                sp.close();
                break;
        }
    }
    return sp;
}

SkRect to_skia_rect(const rect& r) noexcept {
    return SkRect::MakeXYWH(SkDoubleToScalar(r.x),
                            SkDoubleToScalar(r.y),
                            SkDoubleToScalar(r.w),
                            SkDoubleToScalar(r.h));
}

} // namespace

class skia_canvas final : public canvas {
public:
    skia_canvas(int w, int h) : w_{w}, h_{h} {
        // kN32_SkColorType on little-endian is BGRA premultiplied —
        // matching what pixel_data() promises to return.
        bitmap_.allocN32Pixels(w, h, /*isOpaque=*/false);
        bitmap_.eraseColor(SK_ColorTRANSPARENT);
        canvas_ = std::make_unique<SkCanvas>(bitmap_);
    }

    ~skia_canvas() override = default;

    [[nodiscard]] int width_px()  const noexcept override { return w_; }
    [[nodiscard]] int height_px() const noexcept override { return h_; }

    void save()    override { canvas_->save(); }
    void restore() override { canvas_->restore(); }

    void translate(double dx, double dy) override {
        canvas_->translate(SkDoubleToScalar(dx), SkDoubleToScalar(dy));
    }
    void scale(double sx, double sy) override {
        canvas_->scale(SkDoubleToScalar(sx), SkDoubleToScalar(sy));
    }
    void rotate(double radians) override {
        // SkCanvas::rotate takes degrees; convert at the boundary so
        // the facade API stays radian-consistent across backends.
        canvas_->rotate(SkDoubleToScalar(radians * 180.0 / kPi));
    }

    void set_fill(color c)            override { fill_   = c; }
    void set_stroke(color c)          override { stroke_ = c; }
    void set_stroke_width(double w)   override { stroke_width_ = static_cast<float>(w); }
    void set_line_cap(line_cap c)     override { cap_   = c; }
    void set_line_join(line_join j)   override { join_  = j; }
    void set_opacity(double a)        override { opacity_ = static_cast<float>(a); }

    void clear(color c) override {
        canvas_->clear(SkColor4f{c.r, c.g, c.b, c.a}.toSkColor());
    }

    void fill_rect(rect r) override {
        canvas_->drawRect(to_skia_rect(r), fill_paint());
    }
    void stroke_rect(rect r) override {
        canvas_->drawRect(to_skia_rect(r), stroke_paint());
    }

    void fill_ellipse(rect b) override {
        canvas_->drawOval(to_skia_rect(b), fill_paint());
    }
    void stroke_ellipse(rect b) override {
        canvas_->drawOval(to_skia_rect(b), stroke_paint());
    }

    void fill_path(const path& p) override {
        canvas_->drawPath(to_skia_path(p), fill_paint());
    }
    void stroke_path(const path& p) override {
        canvas_->drawPath(to_skia_path(p), stroke_paint());
    }

    void clip(const path& p) override {
        canvas_->clipPath(to_skia_path(p), /*doAntiAlias=*/true);
    }

    // Pixel readback. Skia's allocN32Pixels uses the platform-native
    // 32-bit format — BGRA premultiplied on every little-endian
    // platform MPAPP supports — so this matches the format documented
    // on the abstract canvas API. No copy: the pointer aliases the
    // SkBitmap's owned pixel memory and stays valid until the next
    // realloc (which we don't trigger after construction).
    [[nodiscard]] const std::uint8_t* pixel_data() const noexcept override {
        const void* px = bitmap_.getPixels();
        return static_cast<const std::uint8_t*>(px);
    }
    [[nodiscard]] int pixel_stride_bytes() const noexcept override {
        return static_cast<int>(bitmap_.rowBytes());
    }

private:
    SkPaint fill_paint() const {
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::Style::kFill_Style);
        p.setColor4f(SkColor4f{fill_.r, fill_.g, fill_.b, fill_.a * opacity_}, nullptr);
        return p;
    }
    SkPaint stroke_paint() const {
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::Style::kStroke_Style);
        p.setStrokeWidth(stroke_width_);
        p.setStrokeCap(to_skia_cap(cap_));
        p.setStrokeJoin(to_skia_join(join_));
        p.setColor4f(SkColor4f{stroke_.r, stroke_.g, stroke_.b, stroke_.a * opacity_}, nullptr);
        return p;
    }

    int                       w_{0};
    int                       h_{0};
    SkBitmap                  bitmap_{};
    std::unique_ptr<SkCanvas> canvas_{};

    color     fill_{0.0f, 0.0f, 0.0f, 1.0f};
    color     stroke_{0.0f, 0.0f, 0.0f, 1.0f};
    float     stroke_width_{1.0f};
    line_cap  cap_{line_cap::butt};
    line_join join_{line_join::miter};
    float     opacity_{1.0f};
};

std::unique_ptr<canvas> make_canvas(int width_px, int height_px) {
    return std::make_unique<skia_canvas>(width_px, height_px);
}

} // namespace mpapp::detail::graphics

#endif // MPAPP_GRAPHICS_HAS_SKIA
