// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ScrollView.md
//
// `mpapp::scroll_view` — single-child container with viewport panning.
// Mock surface (P2). Read-only `scroll_x` / `scroll_y` are still
// Observables — the real handler writes to them; the mock test treats
// them as plain observable values.

#ifndef MPAPP_INTERNAL_BASIC_SCROLL_VIEW_HPP
#define MPAPP_INTERNAL_BASIC_SCROLL_VIEW_HPP

#include <cstdint>
#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_SCROLL_VIEW_HAS_STD_FORMAT 1
#endif
#include <memory>
#include <string_view>

#include "../command.hpp"
#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"
#include "../view.hpp"

namespace mpapp {

enum class scroll_orientation : std::uint8_t {
    vertical   = 0,
    horizontal = 1,
    both       = 2,
    neither    = 3,
};

enum class scroll_bar_visibility : std::uint8_t {
    default_visibility = 0,
    always             = 1,
    never              = 2,
};

enum class scroll_to_position : std::uint8_t {
    make_visible = 0,
    start        = 1,
    center       = 2,
    end          = 3,
};

// Argument bundle for the `scroll_to` command. Either x/y are absolute
// pixel offsets, or `element` names a target child + position.
struct scroll_to_request {
    double             x        = 0.0;
    double             y        = 0.0;
    view*              element  = nullptr;
    scroll_to_position position = scroll_to_position::make_visible;
    bool               animated = true;

    bool operator==(const scroll_to_request&) const = default;
};

struct scrolled_args {
    double scroll_x = 0.0;
    double scroll_y = 0.0;
};

} // namespace mpapp

namespace mpapp::internal {

template <class Platform = platform::current>
class scroll_view_handler;

class basic_scroll_view : public view {
public:
    basic_scroll_view() = default;

    Observable<std::shared_ptr<view>>      content{};
    Observable<scroll_orientation>         orientation{scroll_orientation::vertical};
    Observable<scroll_bar_visibility>      horizontal_scroll_bar_visibility{
        scroll_bar_visibility::default_visibility};
    Observable<scroll_bar_visibility>      vertical_scroll_bar_visibility{
        scroll_bar_visibility::default_visibility};

    // Read-only from user code's POV (the handler writes them). Exposed
    // as Observable so XAML one-way bindings can read them.
    Observable<double>                     scroll_x{0.0};
    Observable<double>                     scroll_y{0.0};

    // Command: scroll-to. The XAML compiler lowers `<ScrollView.ScrollTo>`
    // bindings to this method. Body is no-op at the cross-platform layer
    // — the mock handler's mapper records the request.
    void scroll_to(scroll_to_request /*req*/, Command<scroll_to_request> = {}) noexcept {}

    // Fires whenever the offset changes. Real handlers connect this to
    // the native scroll-event; mock tests can fire it directly.
    mpapp::signal<const scrolled_args&>    scrolled;

    scroll_view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const scroll_view_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                          has_handler() const noexcept { return handler_ != nullptr; }
    void                                          set_handler(scroll_view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    scroll_view_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal

namespace mpapp {

constexpr std::string_view to_string(scroll_orientation o) noexcept {
    switch (o) {
        case scroll_orientation::vertical:   return "vertical";
        case scroll_orientation::horizontal: return "horizontal";
        case scroll_orientation::both:       return "both";
        case scroll_orientation::neither:    return "neither";
    }
    return "?";
}

constexpr std::string_view to_string(scroll_bar_visibility v) noexcept {
    switch (v) {
        case scroll_bar_visibility::default_visibility: return "default";
        case scroll_bar_visibility::always:             return "always";
        case scroll_bar_visibility::never:              return "never";
    }
    return "?";
}

} // namespace mpapp


#ifdef MPAPP_SCROLL_VIEW_HAS_STD_FORMAT

template <>
struct std::formatter<mpapp::scroll_orientation> : std::formatter<std::string_view> {
    auto format(mpapp::scroll_orientation o, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(o), ctx);
    }
};

template <>
struct std::formatter<mpapp::scroll_bar_visibility> : std::formatter<std::string_view> {
    auto format(mpapp::scroll_bar_visibility v, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(v), ctx);
    }
};

#endif // MPAPP_SCROLL_VIEW_HAS_STD_FORMAT

#endif // MPAPP_INTERNAL_BASIC_SCROLL_VIEW_HPP
