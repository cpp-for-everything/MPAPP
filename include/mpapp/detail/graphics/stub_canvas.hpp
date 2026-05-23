// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. The default `canvas` backend per
// [[ADR-0015-graphics-backend-dual]].
//
// `mpapp::detail::graphics::stub_canvas` is a no-rendering backend that
// records every method call for inspection. It is the default backend
// when `MPAPP_GRAPHICS_BACKEND` is unset or set to `stub`, and the
// natural test fixture for code that uses the canvas surface (e.g.
// `shape_view_handler`, `graphics_view_handler`) without needing a
// real graphics dependency.
//
// The call records are exposed as a parallel vector of strings so
// tests can do exact-match assertions on the sequence of operations
// without coupling to the concrete value types.

#ifndef MPAPP_DETAIL_GRAPHICS_STUB_CANVAS_HPP
#define MPAPP_DETAIL_GRAPHICS_STUB_CANVAS_HPP

#include <cstdio>
#include <string>
#include <vector>

#include "canvas.hpp"

namespace mpapp::detail::graphics {

class stub_canvas final : public canvas {
public:
    stub_canvas(int w, int h) : w_{w}, h_{h} {}

    // canvas overrides --------------------------------------------------

    [[nodiscard]] int width_px()  const noexcept override { return w_; }
    [[nodiscard]] int height_px() const noexcept override { return h_; }

    void save()    override { record("save"); }
    void restore() override { record("restore"); }

    void translate(double dx, double dy) override {
        record_fmt("translate(%g,%g)", dx, dy);
    }
    void scale(double sx, double sy) override {
        record_fmt("scale(%g,%g)", sx, sy);
    }
    void rotate(double radians) override {
        record_fmt("rotate(%g)", radians);
    }

    void set_fill(color c) override {
        record_fmt("set_fill(%g,%g,%g,%g)", c.r, c.g, c.b, c.a);
    }
    void set_stroke(color c) override {
        record_fmt("set_stroke(%g,%g,%g,%g)", c.r, c.g, c.b, c.a);
    }
    void set_stroke_width(double w) override {
        record_fmt("set_stroke_width(%g)", w);
    }
    void set_line_cap(line_cap c) override {
        record_fmt("set_line_cap(%d)", static_cast<int>(c));
    }
    void set_line_join(line_join j) override {
        record_fmt("set_line_join(%d)", static_cast<int>(j));
    }
    void set_opacity(double a) override {
        record_fmt("set_opacity(%g)", a);
    }

    void clear(color c) override {
        record_fmt("clear(%g,%g,%g,%g)", c.r, c.g, c.b, c.a);
    }
    void fill_rect(rect r) override {
        record_fmt("fill_rect(%g,%g,%g,%g)", r.x, r.y, r.w, r.h);
    }
    void stroke_rect(rect r) override {
        record_fmt("stroke_rect(%g,%g,%g,%g)", r.x, r.y, r.w, r.h);
    }
    void fill_ellipse(rect b) override {
        record_fmt("fill_ellipse(%g,%g,%g,%g)", b.x, b.y, b.w, b.h);
    }
    void stroke_ellipse(rect b) override {
        record_fmt("stroke_ellipse(%g,%g,%g,%g)", b.x, b.y, b.w, b.h);
    }
    void fill_path(const path& p) override {
        record_fmt("fill_path(ops=%zu)", p.size());
    }
    void stroke_path(const path& p) override {
        record_fmt("stroke_path(ops=%zu)", p.size());
    }
    void clip(const path& p) override {
        record_fmt("clip(ops=%zu)", p.size());
    }

    // Stub backend has no real pixel buffer. Returning (nullptr, 0)
    // signals to handlers that pixel-blit isn't available — the
    // handler should fall back to skipping the paint or rendering a
    // placeholder (the stub backend ships with `MPAPP_GRAPHICS_BACKEND=stub`
    // which is the explicitly "no-rendering" choice).
    [[nodiscard]] const std::uint8_t* pixel_data()         const noexcept override { return nullptr; }
    [[nodiscard]] int                 pixel_stride_bytes() const noexcept override { return 0; }

    // Test inspection ---------------------------------------------------

    [[nodiscard]] const std::vector<std::string>& calls() const noexcept { return calls_; }
    void                                          clear_calls()    noexcept { calls_.clear(); }

private:
    void record(const char* s) { calls_.emplace_back(s); }

    // Small format helper — fmtlib isn't a dependency and std::format
    // varies in compiler support. snprintf into a stack buffer is
    // sufficient for the limited shapes of strings we emit here.
    template <class... Args>
    void record_fmt(const char* fmt, Args... args) {
        char buf[128]{};
        const int n = std::snprintf(buf, sizeof(buf), fmt, args...);
        if (n > 0) {
            calls_.emplace_back(buf, static_cast<std::size_t>(
                n < static_cast<int>(sizeof(buf)) ? n : sizeof(buf) - 1));
        }
    }

    int                      w_{0};
    int                      h_{0};
    std::vector<std::string> calls_{};
};

} // namespace mpapp::detail::graphics

#endif // MPAPP_DETAIL_GRAPHICS_STUB_CANVAS_HPP
