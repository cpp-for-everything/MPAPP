// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/BoxView.md
//
// `mpapp::box_view` — solid-colored, optionally-rounded rectangle.
// Mock surface (P2). Default measured size mirrors MAUI: 40 × 40 dip.

#ifndef MPAPP_BOX_VIEW_HPP
#define MPAPP_BOX_VIEW_HPP

#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_BOX_VIEW_HAS_STD_FORMAT 1
#endif

#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

// Independent per-corner radius. MAUI's `CornerRadius` carries four
// values; the mock surface keeps the same shape.
struct corner_radius {
    double top_left     = 0.0;
    double top_right    = 0.0;
    double bottom_left  = 0.0;
    double bottom_right = 0.0;

    bool operator==(const corner_radius&) const = default;
};

// Lightweight color. The full sRGB / wide-gamut `color` type lands in
// P3 alongside the graphics handlers; the mock records the four channels
// as-is.
struct color {
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;
    double a = 1.0;

    bool operator==(const color&) const = default;
};

template <class Platform = platform::current>
class box_view_handler;

class box_view : public view {
public:
    box_view() = default;

    Observable<color>          fill{};            // MAUI: `Color` (distinct from `background`)
    Observable<corner_radius>  corners{};

    box_view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const box_view_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                       has_handler() const noexcept { return handler_ != nullptr; }
    void                                       set_handler(box_view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    box_view_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#ifdef MPAPP_BOX_VIEW_HAS_STD_FORMAT

template <>
struct std::formatter<mpapp::color> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const mpapp::color& c, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "rgba({},{},{},{})", c.r, c.g, c.b, c.a);
    }
};

template <>
struct std::formatter<mpapp::corner_radius> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const mpapp::corner_radius& cr, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "corners({},{},{},{})",
                              cr.top_left, cr.top_right, cr.bottom_left, cr.bottom_right);
    }
};

#endif // MPAPP_BOX_VIEW_HAS_STD_FORMAT

#endif // MPAPP_BOX_VIEW_HPP
