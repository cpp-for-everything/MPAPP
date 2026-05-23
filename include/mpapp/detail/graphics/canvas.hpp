// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. The unified 2D graphics facade per
// [[ADR-0015-graphics-backend-dual]].
//
// `mpapp::detail::graphics::canvas` is the abstract interface that
// `shape_view`, `graphics_view`, and any future custom-drawing
// surface use to render. Backends (Cairo, Skia, stub) implement this
// interface; one backend is selected at compile time via the
// `MPAPP_GRAPHICS_BACKEND` CMake option.
//
// Why a base-class virtual interface (not a concept / template
// strategy)? The canvas crosses translation-unit boundaries — handler
// .cpp files call into `canvas*` without knowing the concrete backend.
// A v-table is the cheapest way to keep handler code backend-agnostic
// while still letting the backend impl pick its own translation unit
// (and license footprint). Per-call dispatch cost is irrelevant
// alongside the actual paint cost.
//
// The surface is deliberately narrow — what ShapeView + GraphicsView
// need today, no more. Future extensions (gradients, image filters,
// path effects) add new virtual methods in a controlled way; the
// existing-binary-compatibility story isn't a concern because we ship
// the backend in the same binary as the framework.

#ifndef MPAPP_DETAIL_GRAPHICS_CANVAS_HPP
#define MPAPP_DETAIL_GRAPHICS_CANVAS_HPP

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace mpapp::detail::graphics {

// ---- Value types ---------------------------------------------------------

// RGBA color in [0, 1] floats. Most callers use the factory helpers
// (`rgb`, `rgba`, `from_hex`) rather than constructing directly.
struct color {
    float r{0.0f};
    float g{0.0f};
    float b{0.0f};
    float a{1.0f};

    static constexpr color rgb(float rr, float gg, float bb) noexcept {
        return color{rr, gg, bb, 1.0f};
    }
    static constexpr color rgba(float rr, float gg, float bb, float aa) noexcept {
        return color{rr, gg, bb, aa};
    }

    // Parses "#RRGGBB" or "#RRGGBBAA" (case-insensitive, '#' optional).
    // Returns a transparent black on parse failure rather than throwing —
    // the canvas surface is hot-path and prefers a sensible default.
    static color from_hex(std::string_view hex) noexcept;

    constexpr bool operator==(const color&) const = default;
};

struct point { double x{0.0}; double y{0.0}; };
struct size  { double w{0.0}; double h{0.0}; };
struct rect  { double x{0.0}; double y{0.0}; double w{0.0}; double h{0.0}; };

// ---- Stroke style enums --------------------------------------------------

enum class line_cap : std::uint8_t {
    butt   = 0,
    round  = 1,
    square = 2,
};

enum class line_join : std::uint8_t {
    miter = 0,
    round = 1,
    bevel = 2,
};

// ---- Path ----------------------------------------------------------------
//
// `path` is a builder-style value type holding the operations needed to
// render an arbitrary 2D curve. Backends consume the op list (vs. e.g.
// keeping a backend-specific handle) so paths are cheap to construct,
// copy, and serialize. The op kinds mirror SVG path commands.

enum class path_op_kind : std::uint8_t {
    move    = 0,
    line    = 1,
    quad    = 2,  // quadratic Bezier — one control point
    cubic   = 3,  // cubic Bezier — two control points
    close   = 4,
};

struct path_op {
    path_op_kind kind{path_op_kind::move};
    // Up to 6 floats for cubic (2 cps + endpoint). Unused slots are
    // ignored per kind:
    //   move  / line  use x[0], y[0]
    //   quad  uses x[0], y[0] (control)  + x[1], y[1] (endpoint)
    //   cubic uses x[0], y[0] + x[1], y[1] (controls) + x[2], y[2] (endpoint)
    //   close uses nothing
    double x[3]{};
    double y[3]{};
};

class path {
public:
    path() = default;

    path& move_to(double x, double y) {
        ops_.push_back(path_op{path_op_kind::move, {x, 0, 0}, {y, 0, 0}});
        return *this;
    }
    path& line_to(double x, double y) {
        ops_.push_back(path_op{path_op_kind::line, {x, 0, 0}, {y, 0, 0}});
        return *this;
    }
    path& quad_to(double cx, double cy, double x, double y) {
        ops_.push_back(path_op{path_op_kind::quad, {cx, x, 0}, {cy, y, 0}});
        return *this;
    }
    path& cubic_to(double cx1, double cy1,
                   double cx2, double cy2,
                   double x, double y) {
        ops_.push_back(path_op{path_op_kind::cubic, {cx1, cx2, x}, {cy1, cy2, y}});
        return *this;
    }
    path& close() {
        ops_.push_back(path_op{path_op_kind::close, {0, 0, 0}, {0, 0, 0}});
        return *this;
    }

    [[nodiscard]] const std::vector<path_op>& ops() const noexcept { return ops_; }
    [[nodiscard]] bool                        empty() const noexcept { return ops_.empty(); }
    [[nodiscard]] std::size_t                 size() const noexcept { return ops_.size(); }

    void clear() noexcept { ops_.clear(); }

    // Parse a subset of SVG path syntax: M / L / Q / C / Z (uppercase
    // absolute coords only — relative-coord lowercase variants and the
    // S / T / A shortcuts are deferred). Returns an empty path on
    // parse failure (same fail-quiet rationale as color::from_hex).
    static path from_svg(std::string_view svg);

private:
    std::vector<path_op> ops_;
};

// ---- Canvas interface ----------------------------------------------------

class canvas {
public:
    virtual ~canvas() = default;

    canvas(const canvas&)            = delete;
    canvas& operator=(const canvas&) = delete;
    canvas(canvas&&)                 = delete;
    canvas& operator=(canvas&&)      = delete;

    // ----- Surface metadata -----
    [[nodiscard]] virtual int width_px()  const noexcept = 0;
    [[nodiscard]] virtual int height_px() const noexcept = 0;

    // ----- State stack -----
    virtual void save()    = 0;
    virtual void restore() = 0;

    // ----- Transforms -----
    virtual void translate(double dx, double dy) = 0;
    virtual void scale(double sx, double sy)     = 0;
    virtual void rotate(double radians)          = 0;

    // ----- Paint state (sticky until changed or restore()d) -----
    virtual void set_fill(color c)            = 0;
    virtual void set_stroke(color c)          = 0;
    virtual void set_stroke_width(double w)   = 0;
    virtual void set_line_cap(line_cap c)     = 0;
    virtual void set_line_join(line_join j)   = 0;
    virtual void set_opacity(double a)        = 0;  // global alpha

    // ----- Draw operations -----
    virtual void clear(color c)                  = 0;
    virtual void fill_rect(rect r)               = 0;
    virtual void stroke_rect(rect r)             = 0;
    virtual void fill_ellipse(rect bounds)       = 0;
    virtual void stroke_ellipse(rect bounds)     = 0;
    virtual void fill_path(const path& p)        = 0;
    virtual void stroke_path(const path& p)      = 0;

    // ----- Clipping -----
    // Restricts subsequent draws to the path. Use save()/restore() to
    // scope the clip.
    virtual void clip(const path& p) = 0;

protected:
    canvas() = default;
};

// ---- Factory -------------------------------------------------------------
//
// Creates a canvas using whichever backend was selected at build time.
// The factory is implemented in exactly one .cpp per build (cairo / skia
// / stub) so the linker picks up just one backend.
//
// `width_px` and `height_px` define the backing surface dimensions.
// For backends that target a pre-existing native surface (e.g. a
// `GdkSurface`), the dimensions are advisory — backends decide what to
// do with them.
std::unique_ptr<canvas> make_canvas(int width_px, int height_px);

} // namespace mpapp::detail::graphics

#endif // MPAPP_DETAIL_GRAPHICS_CANVAS_HPP
