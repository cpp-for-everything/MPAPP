// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. `mpapp::brush` — MAUI-style gradient-brush value types
// shared by the widget surfaces (view background, border brush, etc.).
//
// Provides solid-color, linear-gradient, and radial-gradient brushes as
// plain value types (no virtual dispatch, no allocation beyond the stop
// vector). The `brush` alias is a `std::variant` over all three so call
// sites carry a single field without virtual overhead.
//
// `to_string(const brush&)` and `is_gradient(const brush&)` are free
// helpers; they are defined inline so any translation unit that includes
// this header gets them without a .cpp compilation unit.

#ifndef MPAPP_BRUSHES_BRUSH_HPP
#define MPAPP_BRUSHES_BRUSH_HPP

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <mpapp/color.hpp>

namespace mpapp {

// ── 2-D coordinate (relative or absolute depending on context) ───────────

struct point2 {
    double x = 0.0;
    double y = 0.0;

    bool operator==(const point2&) const = default;
};

// ── Gradient stop ─────────────────────────────────────────────────────────

struct gradient_stop {
    double       offset = 0.0;   // [0.0, 1.0] normalised position along gradient
    mpapp::color color{};

    bool operator==(const gradient_stop&) const = default;
};

// ── Brush types ───────────────────────────────────────────────────────────

struct solid_color_brush {
    mpapp::color color{};

    bool operator==(const solid_color_brush&) const = default;
};

struct linear_gradient_brush {
    point2                      start_point{0.0, 0.0};
    point2                      end_point{1.0, 1.0};
    std::vector<gradient_stop>  stops;

    bool operator==(const linear_gradient_brush&) const = default;
};

// Specifies whether center/radius are interpreted relative to the
// bounding box of the painted region or as absolute device coordinates.
enum class radial_gradient_origin : std::uint8_t {
    relative_to_bounding_box = 0,
    absolute                 = 1,
};

[[nodiscard]] constexpr std::string_view to_string(radial_gradient_origin o) noexcept {
    switch (o) {
        case radial_gradient_origin::relative_to_bounding_box: return "relative_to_bounding_box";
        case radial_gradient_origin::absolute:                  return "absolute";
    }
    return "?";
}

struct radial_gradient_brush {
    point2                      center{0.5, 0.5};
    double                      radius = 0.5;
    std::vector<gradient_stop>  stops;

    bool operator==(const radial_gradient_brush&) const = default;
};

// ── Variant alias ─────────────────────────────────────────────────────────

using brush = std::variant<solid_color_brush, linear_gradient_brush, radial_gradient_brush>;

// ── Free helpers ──────────────────────────────────────────────────────────

// Returns a human-readable description of any brush variant. Useful for
// mock-handler recording and test assertions.
[[nodiscard]] inline std::string to_string(const brush& b) {
    return std::visit([](const auto& v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, solid_color_brush>) {
            return "solid(rgba(" +
                   std::to_string(v.color.r) + "," +
                   std::to_string(v.color.g) + "," +
                   std::to_string(v.color.b) + "," +
                   std::to_string(v.color.a) + "))";
        } else if constexpr (std::is_same_v<T, linear_gradient_brush>) {
            return "linear_gradient(stops=" + std::to_string(v.stops.size()) + ")";
        } else {
            static_assert(std::is_same_v<T, radial_gradient_brush>);
            return "radial_gradient(stops=" + std::to_string(v.stops.size()) + ")";
        }
    }, b);
}

// Returns true iff the brush variant holds a gradient (linear or radial).
[[nodiscard]] inline bool is_gradient(const brush& b) noexcept {
    return std::holds_alternative<linear_gradient_brush>(b) ||
           std::holds_alternative<radial_gradient_brush>(b);
}

} // namespace mpapp

#endif // MPAPP_BRUSHES_BRUSH_HPP
