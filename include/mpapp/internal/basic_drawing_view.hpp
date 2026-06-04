// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/DrawingView.md
//
// `mpapp::internal::basic_drawing_view` — freehand drawing canvas surface.
// Mirrors .NET MAUI CommunityToolkit `DrawingView`: a view that holds an
// ordered list of `drawing_line` values, exposes Observable properties for
// per-session defaults, and fires signals when a stroke begins or ends.
// Mock surface (P2 / ADR-0008).

#ifndef MPAPP_INTERNAL_BASIC_DRAWING_VIEW_HPP
#define MPAPP_INTERNAL_BASIC_DRAWING_VIEW_HPP

#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_DRAWING_VIEW_HAS_STD_FORMAT 1
#endif

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"
#include "../view.hpp"   // mpapp::brush_ref lives here

namespace mpapp {

// A 2-D point in device-independent pixels. Matches CTK's `DrawingLinePoint`.
struct point2d {
    double x = 0.0;
    double y = 0.0;

    bool operator==(const point2d&) const = default;
};

// A single continuous stroke: an ordered sequence of points plus visual
// attributes. Mirrors CTK `DrawingLine`.
struct drawing_line {
    std::vector<point2d> points{};
    mpapp::brush_ref     line_color{};
    double               line_width = 1.0;

    bool operator==(const drawing_line&) const = default;
};

} // namespace mpapp

namespace mpapp::internal {

// Primary template — specialised per platform in
// `mpapp/handlers/<platform>/drawing_view_handler.hpp` and
// `mpapp/handlers/mock/drawing_view_handler.hpp`. Forward-declared so
// `basic_drawing_view` can name it as a non-owning pointer member without
// forcing a circular include.
template <class Platform = platform::current>
class drawing_view_handler;

class basic_drawing_view : public view {
public:
    basic_drawing_view() = default;

    basic_drawing_view(const basic_drawing_view&)            = delete;
    basic_drawing_view& operator=(const basic_drawing_view&) = delete;
    basic_drawing_view(basic_drawing_view&&)                 = delete;
    basic_drawing_view& operator=(basic_drawing_view&&)      = delete;

    // ----- Observable properties -------------------------------------------

    // Color applied to new strokes when no per-line color is set.
    Observable<mpapp::brush_ref> default_line_color{};

    // Width (dip) applied to new strokes when no per-line width is set.
    Observable<double> default_line_width{1.0};

    // When true, multiple strokes coexist; when false each new stroke
    // replaces the previous one. Mirrors CTK's `IsMultiLineModeEnabled`.
    Observable<bool> is_multi_line_mode{false};

    // ----- Stroke collection -----------------------------------------------

    // Append a completed stroke to the line list.
    void add_line(drawing_line line) {
        lines_.push_back(std::move(line));
    }

    // Remove all strokes.
    void clear() noexcept {
        lines_.clear();
    }

    // Number of strokes currently stored.
    [[nodiscard]] std::size_t line_count() const noexcept {
        return lines_.size();
    }

    // Access a stroke by index. Throws `std::out_of_range` for bad index.
    [[nodiscard]] const drawing_line& line_at(std::size_t index) const {
        return lines_.at(index);
    }

    // ----- Signals ---------------------------------------------------------

    // Emitted when the user (or test harness) begins a new stroke.
    mpapp::signal<> drawing_started{};

    // Emitted when a stroke is completed. Carries the completed line.
    mpapp::signal<const mpapp::drawing_line&> drawing_line_completed{};

    // ----- Handler attachment (pointer-based, opt-in) ----------------------

    drawing_view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const drawing_view_handler<platform::current>& handler() const noexcept { return *handler_; }
    [[nodiscard]] bool has_handler() const noexcept { return handler_ != nullptr; }
    void set_handler(drawing_view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    std::vector<drawing_line>               lines_{};
    drawing_view_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal

#ifdef MPAPP_DRAWING_VIEW_HAS_STD_FORMAT

template <>
struct std::formatter<mpapp::point2d> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const mpapp::point2d& p, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "({},{})", p.x, p.y);
    }
};

template <>
struct std::formatter<mpapp::drawing_line> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const mpapp::drawing_line& l, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "drawing_line(pts={},color={},w={})",
                              l.points.size(), l.line_color.name, l.line_width);
    }
};

#endif // MPAPP_DRAWING_VIEW_HAS_STD_FORMAT

#endif // MPAPP_INTERNAL_BASIC_DRAWING_VIEW_HPP
