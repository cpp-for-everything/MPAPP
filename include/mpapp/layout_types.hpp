// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/StackLayout.md
//
// Framework-owned layout enum types. User code names these instead of
// platform-native enums (`muxc::Orientation`, GTK `GtkOrientation`,
// `UIStackViewAlignment`, …) so a single .cpp compiles unchanged on
// every supported platform. Each handler translates to / from the
// native enum in its property mappers.
//
// `thickness` lives in <mpapp/layout.hpp> for historical reasons — it
// was introduced with the abstract `layout` container in the P2 mock
// surface.

#ifndef MPAPP_LAYOUT_TYPES_HPP
#define MPAPP_LAYOUT_TYPES_HPP

#include <cstdint>
#include <format>
#include <string_view>

namespace mpapp {

// Stack direction. Mirrors `Microsoft::UI::Xaml::Controls::Orientation`,
// `GtkOrientation`, `UILayoutConstraintAxis`, `LinearLayoutManager.VERTICAL`.
enum class orientation : std::uint8_t {
    vertical   = 0,
    horizontal = 1,
};

// Horizontal child placement within its layout slot. Mirrors MAUI's
// `LayoutOptions` and WinUI's `HorizontalAlignment`.
enum class h_align : std::uint8_t {
    start   = 0,
    center  = 1,
    end     = 2,
    stretch = 3,
};

// Vertical child placement within its layout slot. Mirrors MAUI's
// `LayoutOptions` and WinUI's `VerticalAlignment`.
enum class v_align : std::uint8_t {
    start   = 0,
    center  = 1,
    end     = 2,
    stretch = 3,
};

namespace detail {

constexpr std::string_view orientation_name(orientation o) noexcept {
    switch (o) {
        case orientation::vertical:   return "vertical";
        case orientation::horizontal: return "horizontal";
    }
    return "?";
}

constexpr std::string_view h_align_name(h_align a) noexcept {
    switch (a) {
        case h_align::start:   return "start";
        case h_align::center:  return "center";
        case h_align::end:     return "end";
        case h_align::stretch: return "stretch";
    }
    return "?";
}

constexpr std::string_view v_align_name(v_align a) noexcept {
    switch (a) {
        case v_align::start:   return "start";
        case v_align::center:  return "center";
        case v_align::end:     return "end";
        case v_align::stretch: return "stretch";
    }
    return "?";
}

} // namespace detail

} // namespace mpapp

template <>
struct std::formatter<mpapp::orientation> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(mpapp::orientation o, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", mpapp::detail::orientation_name(o));
    }
};

template <>
struct std::formatter<mpapp::h_align> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(mpapp::h_align a, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", mpapp::detail::h_align_name(a));
    }
};

template <>
struct std::formatter<mpapp::v_align> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(mpapp::v_align a, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", mpapp::detail::v_align_name(a));
    }
};

#endif // MPAPP_LAYOUT_TYPES_HPP
