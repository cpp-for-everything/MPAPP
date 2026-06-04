// SPDX-License-Identifier: Apache-2.0
// `mpapp::two_pane_view` — dual-screen / dual-pane container. Mirrors the
// .NET MAUI CommunityToolkit TwoPaneView: two content slots (pane1/pane2)
// plus Observable properties for adaptive-layout mode, pane priority, and
// the breakpoint thresholds that trigger wide/tall mode.

#ifndef MPAPP_INTERNAL_BASIC_TWO_PANE_VIEW_HPP
#define MPAPP_INTERNAL_BASIC_TWO_PANE_VIEW_HPP

#include <cstdint>
#include <memory>
#include <string_view>
#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_TWO_PANE_VIEW_HAS_STD_FORMAT 1
#endif

#include "../observable.hpp"
#include "../platform.hpp"
#include "../view.hpp"

namespace mpapp {

// Layout mode chosen by the framework based on available screen geometry.
enum class two_pane_mode : std::uint8_t {
    single_pane = 0, // only one pane is visible (narrow viewport or folded)
    wide        = 1, // panes arranged side-by-side (horizontal split)
    tall        = 2, // panes stacked vertically
};

// Which pane is treated as the primary / dominant pane when only one can
// be shown in single_pane mode, or when space is constrained.
enum class two_pane_priority : std::uint8_t {
    pane1 = 0,
    pane2 = 1,
};

constexpr std::string_view to_string(two_pane_mode m) noexcept {
    switch (m) {
        case two_pane_mode::single_pane: return "single_pane";
        case two_pane_mode::wide:        return "wide";
        case two_pane_mode::tall:        return "tall";
    }
    return "?";
}

constexpr std::string_view to_string(two_pane_priority p) noexcept {
    switch (p) {
        case two_pane_priority::pane1: return "pane1";
        case two_pane_priority::pane2: return "pane2";
    }
    return "?";
}

} // namespace mpapp

namespace mpapp::internal {

template <class Platform = platform::current>
class two_pane_view_handler;

class basic_two_pane_view : public view {
public:
    basic_two_pane_view() = default;

    // ----- Content slots ---------------------------------------------------
    // The two child panes. Held as shared_ptr<view> so ownership is clear
    // and the pane can be shared with a view-model binding. nullptr = empty.
    [[nodiscard]] std::shared_ptr<view> pane1() const noexcept { return pane1_; }
    [[nodiscard]] std::shared_ptr<view> pane2() const noexcept { return pane2_; }

    void set_pane1(std::shared_ptr<view> p) {
        if (p == pane1_) return;
        pane1_ = std::move(p);
        pane1_changed.emit(pane1_);
    }

    void set_pane2(std::shared_ptr<view> p) {
        if (p == pane2_) return;
        pane2_ = std::move(p);
        pane2_changed.emit(pane2_);
    }

    // Signals emitted when a pane slot changes (for handler subscriptions).
    mpapp::signal<const std::shared_ptr<view>&> pane1_changed{};
    mpapp::signal<const std::shared_ptr<view>&> pane2_changed{};

    // ----- Adaptive-layout properties -------------------------------------
    Observable<two_pane_mode>     mode{two_pane_mode::single_pane};
    Observable<two_pane_priority> panel_priority{two_pane_priority::pane1};

    // Minimum viewport width that triggers wide mode (device-independent units).
    Observable<double> min_wide_mode_width{641.0};
    // Minimum viewport height that triggers tall mode (device-independent units).
    Observable<double> min_tall_mode_height{641.0};

    // ----- Handler --------------------------------------------------------
    two_pane_view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const two_pane_view_handler<platform::current>& handler() const noexcept { return *handler_; }
    [[nodiscard]] bool has_handler() const noexcept { return handler_ != nullptr; }
    void set_handler(two_pane_view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    std::shared_ptr<view>                      pane1_{};
    std::shared_ptr<view>                      pane2_{};
    two_pane_view_handler<platform::current>*  handler_ = nullptr;
};

} // namespace mpapp::internal

#ifdef MPAPP_TWO_PANE_VIEW_HAS_STD_FORMAT

template <>
struct std::formatter<mpapp::two_pane_mode> : std::formatter<std::string_view> {
    auto format(mpapp::two_pane_mode m, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(m), ctx);
    }
};

template <>
struct std::formatter<mpapp::two_pane_priority> : std::formatter<std::string_view> {
    auto format(mpapp::two_pane_priority p, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(p), ctx);
    }
};

#endif // MPAPP_TWO_PANE_VIEW_HAS_STD_FORMAT

#endif // MPAPP_INTERNAL_BASIC_TWO_PANE_VIEW_HPP
