// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/View.md
//
// `mpapp::view` — cross-platform base of every visible widget. Mirrors
// MAUI's `IView` / `ViewHandler.ViewMapper` cross-cutting property
// surface (identity, layout, visual state, transforms, hit-testing).
//
// This is the **mock surface** (P2 / ADR-0008): every property is wired
// up as an `Observable<T>`, every command is a `Command<>` tag, and a
// handler reference is stored non-owning. Sets are forwarded to the
// platform handler via `notify_<prop>()` helpers that exist only so the
// mock handler can record them — real handlers route the same way.
//
// For the mock surface the rich types (`brush_ref`, `shadow`,
// `semantics`, `geometry_ref`) are reduced to lightweight stand-ins
// (`std::string` for resource names, plain enums for state). The full
// types land alongside their real handlers in P3+.

#ifndef MPAPP_VIEW_HPP
#define MPAPP_VIEW_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_VIEW_HAS_STD_FORMAT 1
#endif

#include "command.hpp"
#include "observable.hpp"
#include "platform.hpp"

namespace mpapp {

// Lightweight stand-ins for the rich types the full View surface will
// carry. Defined here so every layout-group mock can use them without a
// dedicated header per primitive. The real geometry / brush / semantics
// types replace these in P3 — the property names and units stay the same.

enum class visibility : std::uint8_t {
    visible   = 0,
    hidden    = 1,
    collapsed = 2,
};

enum class flow_direction : std::uint8_t {
    match_parent = 0,
    left_to_right = 1,
    right_to_left = 2,
};

// A bitmask of edges the framework treats as safe-area boundaries. The
// mock surface stores it as a plain `std::uint8_t` — bit 0 = top, bit 1
// = right, bit 2 = bottom, bit 3 = left. The full `safe_area_edges`
// enum / struct lands with the real handlers.
using safe_area_edges_mask = std::uint8_t;

// Symbolic brush reference. Real implementations replace this with a
// `brush_ref` variant over solid / linear / radial brushes; the mock
// records the string repr.
struct brush_ref {
    std::string name;

    bool operator==(const brush_ref&) const = default;
};

// Symbolic shadow descriptor — enough for tests to observe "shadow set".
struct shadow_desc {
    double offset_x = 0.0;
    double offset_y = 0.0;
    double radius   = 0.0;
    double opacity  = 0.0;

    bool operator==(const shadow_desc&) const = default;
};

// Forward-declared so the cross-platform header doesn't pull in the mock
// (or any real) backend. Each platform specialises this template under
// `mpapp/handlers/<platform>/view_handler.hpp`.
template <class Platform = platform::current>
class view_handler;

class view {
public:
    view() = default;
    virtual ~view() = default;

    view(const view&)            = delete;
    view& operator=(const view&) = delete;
    view(view&&)                 = delete;
    view& operator=(view&&)      = delete;

    // ----- Identity / accessibility -------------------------------------
    Observable<std::string>                 automation_id{""};
    Observable<std::optional<std::string>>  tool_tip{std::nullopt};

    // ----- Layout -------------------------------------------------------
    Observable<double>                      width{-1.0};          // -1 = unset / auto
    Observable<double>                      height{-1.0};
    Observable<double>                      minimum_width{0.0};
    Observable<double>                      minimum_height{0.0};
    Observable<double>                      maximum_width{0.0};
    Observable<double>                      maximum_height{0.0};
    Observable<flow_direction>              flow{flow_direction::match_parent};
    Observable<safe_area_edges_mask>        safe_area_edges{0};

    // ----- Visual state -------------------------------------------------
    Observable<visibility>                  visibility_state{visibility::visible};
    Observable<bool>                        is_enabled{true};
    Observable<double>                      opacity{1.0};
    Observable<brush_ref>                   background{};
    Observable<shadow_desc>                 shadow{};

    // ----- Transforms ---------------------------------------------------
    Observable<double>                      translation_x{0.0};
    Observable<double>                      translation_y{0.0};
    Observable<double>                      scale{1.0};
    Observable<double>                      scale_x{1.0};
    Observable<double>                      scale_y{1.0};
    Observable<double>                      rotation{0.0};
    Observable<double>                      rotation_x{0.0};
    Observable<double>                      rotation_y{0.0};
    Observable<double>                      anchor_x{0.5};
    Observable<double>                      anchor_y{0.5};
    Observable<int>                         z_index{0};

    // ----- Hit testing --------------------------------------------------
    Observable<bool>                        input_transparent{false};

    // ----- Commands -----------------------------------------------------
    // Declared with the Command<> tag per ADR-0009. The XAML compiler
    // recognises the tag and lowers `Command="…"` bindings to direct
    // calls. The mock surface keeps the bodies trivial — the framework
    // owns the routing-to-handler concern and lands with the M-03
    // command mapper plumbing.
    void invalidate_measure(Command<> = {}) noexcept {}
    void focus(Command<>            = {}) noexcept {}
    void unfocus(Command<>          = {}) noexcept {}

    // ----- Handler ------------------------------------------------------
    view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const view_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                   has_handler() const noexcept { return handler_ != nullptr; }
    void                                   set_handler(view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    view_handler<platform::current>* handler_ = nullptr;
};

// Stable string repr for the mock-recording layer. Defined inline so any
// translation unit that includes <mpapp/view.hpp> gets a consistent
// std::format spelling for the enums — tests assert on these strings.

constexpr std::string_view to_string(visibility v) noexcept {
    switch (v) {
        case visibility::visible:   return "visible";
        case visibility::hidden:    return "hidden";
        case visibility::collapsed: return "collapsed";
    }
    return "?";
}

constexpr std::string_view to_string(flow_direction f) noexcept {
    switch (f) {
        case flow_direction::match_parent:  return "match_parent";
        case flow_direction::left_to_right: return "ltr";
        case flow_direction::right_to_left: return "rtl";
    }
    return "?";
}

} // namespace mpapp

// std::formatter specialisations — keep the recording stable across
// platforms / locales / compilers. They live with the type so any
// `std::format("{}", value)` call site (mock handler or user code) gets
// the same string.

#ifdef MPAPP_VIEW_HAS_STD_FORMAT

template <>
struct std::formatter<mpapp::visibility> : std::formatter<std::string_view> {
    auto format(mpapp::visibility v, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(v), ctx);
    }
};

template <>
struct std::formatter<mpapp::flow_direction> : std::formatter<std::string_view> {
    auto format(mpapp::flow_direction f, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(f), ctx);
    }
};

template <>
struct std::formatter<mpapp::brush_ref> : std::formatter<std::string_view> {
    auto format(const mpapp::brush_ref& b, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(b.name, ctx);
    }
};

template <>
struct std::formatter<mpapp::shadow_desc> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const mpapp::shadow_desc& s, std::format_context& ctx) const {
        return std::format_to(ctx.out(),
                              "shadow(dx={},dy={},r={},a={})",
                              s.offset_x, s.offset_y, s.radius, s.opacity);
    }
};

#endif // MPAPP_VIEW_HAS_STD_FORMAT

#endif // MPAPP_VIEW_HPP
