// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. `mpapp::color` — the lightweight public sRGB color value
// type shared by the widget surfaces (box_view fill, label text color, …).
//
// Distinct from the graphics-backend `mpapp::detail::graphics::color`
// (float channels, used by the canvas facade). This one keeps double
// channels to match the MAUI `Color` shape and the existing box_view
// surface that first introduced it.

#ifndef MPAPP_COLOR_HPP
#define MPAPP_COLOR_HPP

#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_COLOR_HAS_STD_FORMAT 1
#endif

namespace mpapp {

struct color {
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;
    double a = 1.0;

    bool operator==(const color&) const = default;

    // Convenience constructor from 0–255 sRGB bytes (alpha 0–255).
    static constexpr color from_rgb8(int rr, int gg, int bb, int aa = 255) noexcept {
        return color{rr / 255.0, gg / 255.0, bb / 255.0, aa / 255.0};
    }
};

} // namespace mpapp

#ifdef MPAPP_COLOR_HAS_STD_FORMAT
template <>
struct std::formatter<mpapp::color> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const mpapp::color& c, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "rgba({},{},{},{})", c.r, c.g, c.b, c.a);
    }
};
#endif

#endif // MPAPP_COLOR_HPP
