// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MapView.md
//
// `mpapp::basic_map_view` — interactive map surface. Mirrors the
// .NET MAUI Map / CommunityToolkit .NET MAUI Map control surface.
// Mock surface (P2). Real platform map SDK binding is a follow-up.

#ifndef MPAPP_INTERNAL_BASIC_MAP_VIEW_HPP
#define MPAPP_INTERNAL_BASIC_MAP_VIEW_HPP

#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_MAP_VIEW_HAS_STD_FORMAT 1
#endif

#include <cstdint>
#include <string>
#include <vector>

#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"
#include "../view.hpp"

namespace mpapp {

// Geographic coordinate pair (WGS-84). Mirrors MAUI's `Location`.
struct geo_point {
    double latitude  = 0.0;
    double longitude = 0.0;

    bool operator==(const geo_point&) const = default;
};

// A marker placed on the map. Mirrors MAUI's `Pin`.
struct map_pin {
    std::string label;
    std::string address;
    geo_point   location;

    bool operator==(const map_pin&) const = default;
};

// Map tile style. Mirrors MAUI's `MapType`.
enum class map_kind : std::uint8_t {
    street    = 0,
    satellite = 1,
    hybrid    = 2,
};

constexpr std::string_view to_string(map_kind k) noexcept {
    switch (k) {
        case map_kind::street:    return "street";
        case map_kind::satellite: return "satellite";
        case map_kind::hybrid:    return "hybrid";
    }
    return "?";
}

} // namespace mpapp

namespace mpapp::internal {

template <class Platform = platform::current>
class map_view_handler;

class basic_map_view : public view {
public:
    basic_map_view() = default;

    basic_map_view(const basic_map_view&)            = delete;
    basic_map_view& operator=(const basic_map_view&) = delete;
    basic_map_view(basic_map_view&&)                 = delete;
    basic_map_view& operator=(basic_map_view&&)      = delete;

    // ----- Map properties ---------------------------------------------------
    Observable<map_kind>  kind{map_kind::street};
    Observable<geo_point> center{};
    Observable<double>    zoom{1.0};

    // ----- Feature toggles --------------------------------------------------
    Observable<bool>      is_showing_user{false};
    Observable<bool>      is_traffic_enabled{false};
    Observable<bool>      is_scroll_enabled{true};
    Observable<bool>      is_zoom_enabled{true};

    // ----- Pin collection ---------------------------------------------------
    void add_pin(map_pin p) { pins_.push_back(std::move(p)); }
    void clear_pins() noexcept { pins_.clear(); }

    [[nodiscard]] std::size_t pin_count() const noexcept { return pins_.size(); }

    [[nodiscard]] const map_pin& pin_at(std::size_t index) const {
        return pins_.at(index);
    }

    // ----- Signals ----------------------------------------------------------
    // Fired when the user taps a pin. Carries the tapped pin.
    mutable signal<const map_pin&>   pin_clicked;
    // Fired when the user taps on the map (not a pin). Carries the location.
    mutable signal<const geo_point&> map_clicked;

    // ----- Handler ----------------------------------------------------------
    map_view_handler<platform::current>&       handler() noexcept
        { return *handler_; }
    const map_view_handler<platform::current>& handler() const noexcept
        { return *handler_; }
    [[nodiscard]] bool has_handler() const noexcept { return handler_ != nullptr; }
    void set_handler(map_view_handler<platform::current>& h) noexcept
        { handler_ = &h; }

private:
    std::vector<map_pin>                    pins_{};
    map_view_handler<platform::current>*    handler_ = nullptr;
};

} // namespace mpapp::internal


#ifdef MPAPP_MAP_VIEW_HAS_STD_FORMAT

template <>
struct std::formatter<mpapp::map_kind> : std::formatter<std::string_view> {
    auto format(mpapp::map_kind k, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(k), ctx);
    }
};

template <>
struct std::formatter<mpapp::geo_point> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const mpapp::geo_point& gp, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "geo({},{})", gp.latitude, gp.longitude);
    }
};

template <>
struct std::formatter<mpapp::map_pin> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const mpapp::map_pin& p, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "pin({},{},geo({},{}))",
                              p.label, p.address,
                              p.location.latitude, p.location.longitude);
    }
};

#endif // MPAPP_MAP_VIEW_HAS_STD_FORMAT

#endif // MPAPP_INTERNAL_BASIC_MAP_VIEW_HPP
