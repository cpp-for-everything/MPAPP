// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Border.md
//
// `mpapp::border` — single-child decorator drawing a stroke + fill +
// optional non-rectangular outline. Supersedes `mpapp::frame` (MAUI 9
// deprecation parity). Mock surface (P2): `stroke_shape` and the brush
// types are placeholders (`std::string` shape descriptor, `brush_ref`
// from view.hpp). Real types arrive in P3 with the graphics handlers.

#ifndef MPAPP_BORDER_HPP
#define MPAPP_BORDER_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_BORDER_HAS_STD_FORMAT 1
#endif

#include "layout.hpp"   // for `thickness`
#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

enum class pen_line_cap : std::uint8_t {
    flat   = 0,
    round  = 1,
    square = 2,
};

enum class pen_line_join : std::uint8_t {
    miter = 0,
    round = 1,
    bevel = 2,
};

// Textual shape descriptor — e.g. `"Rectangle"`, `"RoundRectangle(12)"`,
// `"Ellipse"`. Real handlers parse the shape from a `shape` type tree.
struct stroke_shape_desc {
    std::string descriptor{"Rectangle"};

    bool operator==(const stroke_shape_desc&) const = default;
};

template <class Platform>
class border_handler;

class border : public view {
public:
    border() = default;

    Observable<std::shared_ptr<view>>   content{};
    Observable<thickness>               padding{};

    Observable<stroke_shape_desc>       stroke_shape{};
    Observable<brush_ref>               stroke{};
    Observable<double>                  stroke_thickness{1.0};
    Observable<std::vector<double>>     stroke_dash_array{};
    Observable<double>                  stroke_dash_offset{0.0};
    Observable<pen_line_cap>            stroke_line_cap{pen_line_cap::flat};
    Observable<pen_line_join>           stroke_line_join{pen_line_join::miter};
    Observable<double>                  stroke_miter_limit{10.0};

    border_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const border_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                     has_handler() const noexcept { return handler_ != nullptr; }
    void                                     set_handler(border_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    border_handler<platform::current>* handler_ = nullptr;
};

constexpr std::string_view to_string(pen_line_cap c) noexcept {
    switch (c) {
        case pen_line_cap::flat:   return "flat";
        case pen_line_cap::round:  return "round";
        case pen_line_cap::square: return "square";
    }
    return "?";
}

constexpr std::string_view to_string(pen_line_join j) noexcept {
    switch (j) {
        case pen_line_join::miter: return "miter";
        case pen_line_join::round: return "round";
        case pen_line_join::bevel: return "bevel";
    }
    return "?";
}

} // namespace mpapp

#ifdef MPAPP_BORDER_HAS_STD_FORMAT

template <>
struct std::formatter<mpapp::pen_line_cap> : std::formatter<std::string_view> {
    auto format(mpapp::pen_line_cap c, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(c), ctx);
    }
};

template <>
struct std::formatter<mpapp::pen_line_join> : std::formatter<std::string_view> {
    auto format(mpapp::pen_line_join j, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(j), ctx);
    }
};

template <>
struct std::formatter<mpapp::stroke_shape_desc> : std::formatter<std::string_view> {
    auto format(const mpapp::stroke_shape_desc& s, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(s.descriptor, ctx);
    }
};

#endif // MPAPP_BORDER_HAS_STD_FORMAT

#endif // MPAPP_BORDER_HPP
