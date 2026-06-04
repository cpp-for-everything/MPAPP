// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/FlexLayout.md
//
// `mpapp::flex_layout` — CSS-flexbox-style layout container, mirroring
// .NET MAUI `FlexLayout`. Container properties (direction, wrap,
// justify_content, align_items, align_content, position) plus per-child
// attached properties (order, grow, shrink, align_self, basis). Native
// rendering: Windows custom Panel, Linux a flexbox-emulating widget,
// Android a flexbox container. Each platform's native container performs
// the actual measure + arrange — MPAPP just maps the flex properties to
// the native API.

#ifndef MPAPP_INTERNAL_BASIC_FLEX_LAYOUT_HPP
#define MPAPP_INTERNAL_BASIC_FLEX_LAYOUT_HPP

#include <cstdint>
#include <string_view>
#include <unordered_map>
#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_FLEX_HAS_STD_FORMAT 1
#endif

#include "../layout.hpp"
#include "../observable.hpp"
#include "../platform.hpp"
#include "../view.hpp"

namespace mpapp {

// Main-axis direction. Mirrors MAUI `FlexDirection`.
enum class flex_direction : std::uint8_t {
    row            = 0,
    row_reverse    = 1,
    column         = 2,
    column_reverse = 3,
};

// Whether children wrap onto multiple lines. Mirrors MAUI `FlexWrap`.
enum class flex_wrap : std::uint8_t {
    no_wrap      = 0,
    wrap         = 1,
    wrap_reverse = 2,
};

// Main-axis distribution of children. Mirrors MAUI `FlexJustify`.
enum class flex_justify : std::uint8_t {
    start         = 0,
    center        = 1,
    end           = 2,
    space_between = 3,
    space_around  = 4,
    space_evenly  = 5,
};

// Cross-axis alignment of children within a line. Mirrors MAUI
// `FlexAlignItems`.
enum class flex_align_items : std::uint8_t {
    stretch = 0,
    center  = 1,
    start   = 2,
    end     = 3,
};

// Cross-axis distribution of lines (multi-line only). Mirrors MAUI
// `FlexAlignContent`.
enum class flex_align_content : std::uint8_t {
    stretch       = 0,
    center        = 1,
    start         = 2,
    end           = 3,
    space_between = 4,
    space_around  = 5,
};

// Per-child cross-axis alignment override. Mirrors MAUI `FlexAlignSelf`.
enum class flex_align_self : std::uint8_t {
    auto_   = 0,
    stretch = 1,
    center  = 2,
    start   = 3,
    end     = 4,
};

// Child positioning mode. Mirrors MAUI `FlexPosition`.
enum class flex_position : std::uint8_t {
    relative = 0,
    absolute = 1,
};

constexpr std::string_view to_string(flex_direction d) noexcept {
    switch (d) {
        case flex_direction::row:            return "row";
        case flex_direction::row_reverse:    return "row_reverse";
        case flex_direction::column:         return "column";
        case flex_direction::column_reverse: return "column_reverse";
    }
    return "?";
}

constexpr std::string_view to_string(flex_wrap w) noexcept {
    switch (w) {
        case flex_wrap::no_wrap:      return "no_wrap";
        case flex_wrap::wrap:         return "wrap";
        case flex_wrap::wrap_reverse: return "wrap_reverse";
    }
    return "?";
}

constexpr std::string_view to_string(flex_justify j) noexcept {
    switch (j) {
        case flex_justify::start:         return "start";
        case flex_justify::center:        return "center";
        case flex_justify::end:           return "end";
        case flex_justify::space_between: return "space_between";
        case flex_justify::space_around:  return "space_around";
        case flex_justify::space_evenly:  return "space_evenly";
    }
    return "?";
}

constexpr std::string_view to_string(flex_align_items a) noexcept {
    switch (a) {
        case flex_align_items::stretch: return "stretch";
        case flex_align_items::center:  return "center";
        case flex_align_items::start:   return "start";
        case flex_align_items::end:     return "end";
    }
    return "?";
}

constexpr std::string_view to_string(flex_align_content a) noexcept {
    switch (a) {
        case flex_align_content::stretch:       return "stretch";
        case flex_align_content::center:        return "center";
        case flex_align_content::start:         return "start";
        case flex_align_content::end:           return "end";
        case flex_align_content::space_between: return "space_between";
        case flex_align_content::space_around:  return "space_around";
    }
    return "?";
}

constexpr std::string_view to_string(flex_align_self a) noexcept {
    switch (a) {
        case flex_align_self::auto_:   return "auto";
        case flex_align_self::stretch: return "stretch";
        case flex_align_self::center:  return "center";
        case flex_align_self::start:   return "start";
        case flex_align_self::end:     return "end";
    }
    return "?";
}

constexpr std::string_view to_string(flex_position p) noexcept {
    switch (p) {
        case flex_position::relative: return "relative";
        case flex_position::absolute: return "absolute";
    }
    return "?";
}

} // namespace mpapp

namespace mpapp::internal {

template <class Platform = platform::current>
class flex_layout_handler;

class basic_flex_layout : public layout {
public:
    basic_flex_layout() = default;
    ~basic_flex_layout() override = default;

    basic_flex_layout(const basic_flex_layout&)            = delete;
    basic_flex_layout& operator=(const basic_flex_layout&) = delete;
    basic_flex_layout(basic_flex_layout&&)                 = delete;
    basic_flex_layout& operator=(basic_flex_layout&&)      = delete;

    // ----- Container properties ---------------------------------------
    Observable<flex_direction>     direction{flex_direction::row};
    Observable<flex_wrap>          wrap{flex_wrap::no_wrap};
    Observable<flex_justify>       justify_content{flex_justify::start};
    Observable<flex_align_items>   align_items{flex_align_items::stretch};
    Observable<flex_align_content> align_content{flex_align_content::stretch};
    Observable<flex_position>      position{flex_position::relative};

    // ----- Per-child attached properties ------------------------------
    //
    // MAUI: FlexLayout.SetOrder(child, 1); FlexLayout.SetGrow(child, 1); ...
    // MPAPP: flex.set_order(child, 1); flex.set_grow(child, 1); ...
    //
    // Storage is a side map keyed on the child's view*. The flex layout's
    // real handler reads these in its add_child path to drive the native
    // per-child API.

    struct child_props {
        int             order     = 0;
        double          grow      = 0.0;
        double          shrink    = 1.0;
        flex_align_self align_self = flex_align_self::auto_;
        // -1 means "auto" (size to content); >= 0 is a literal basis.
        double          basis     = -1.0;
    };

    void set_order     (view& v, int o)             { props_for(v).order = o; }
    void set_grow      (view& v, double g)          { props_for(v).grow = g; }
    void set_shrink    (view& v, double s)          { props_for(v).shrink = s; }
    void set_align_self(view& v, flex_align_self a) { props_for(v).align_self = a; }
    void set_basis     (view& v, double b)          { props_for(v).basis = b; }

    [[nodiscard]] child_props get_child_props(const view& v) const {
        auto it = props_.find(const_cast<view*>(&v));
        return (it == props_.end()) ? child_props{} : it->second;
    }

    flex_layout_handler<platform::current>&       handler() noexcept       { return *flex_handler_; }
    const flex_layout_handler<platform::current>& handler() const noexcept { return *flex_handler_; }
    bool                                          has_handler() const noexcept { return flex_handler_ != nullptr; }
    void                                          set_handler(flex_layout_handler<platform::current>& h) noexcept { flex_handler_ = &h; }

private:
    child_props& props_for(view& v) {
        return props_[&v];
    }

    flex_layout_handler<platform::current>* flex_handler_ = nullptr;
    // Attached-property store. Keyed on the child view*; cleared by the
    // application before the child's lifetime ends (mirrors the
    // basic_grid_layout placement store pattern).
    std::unordered_map<view*, child_props> props_{};
};

} // namespace mpapp::internal

#ifdef MPAPP_FLEX_HAS_STD_FORMAT

template <>
struct std::formatter<mpapp::flex_direction> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(mpapp::flex_direction d, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", mpapp::to_string(d));
    }
};

template <>
struct std::formatter<mpapp::flex_wrap> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(mpapp::flex_wrap w, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", mpapp::to_string(w));
    }
};

template <>
struct std::formatter<mpapp::flex_justify> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(mpapp::flex_justify j, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", mpapp::to_string(j));
    }
};

template <>
struct std::formatter<mpapp::flex_align_items> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(mpapp::flex_align_items a, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", mpapp::to_string(a));
    }
};

template <>
struct std::formatter<mpapp::flex_align_content> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(mpapp::flex_align_content a, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", mpapp::to_string(a));
    }
};

template <>
struct std::formatter<mpapp::flex_align_self> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(mpapp::flex_align_self a, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", mpapp::to_string(a));
    }
};

template <>
struct std::formatter<mpapp::flex_position> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(mpapp::flex_position p, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", mpapp::to_string(p));
    }
};

#endif // MPAPP_FLEX_HAS_STD_FORMAT

#endif // MPAPP_INTERNAL_BASIC_FLEX_LAYOUT_HPP
