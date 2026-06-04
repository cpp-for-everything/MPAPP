// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/AbsoluteLayout.md
//
// `mpapp::absolute_layout` — explicit-positioning layout container. Each
// child carries an attached `layout_bounds` rectangle and a
// `layout_flags` bitmask selecting which bound components are interpreted
// proportionally (0..1 of the container) versus as absolute pixels.
// Mirrors .NET MAUI `AbsoluteLayout`. Native rendering: Windows
// `mux::Controls::Canvas`, Linux `GtkFixed`, Android
// `android.widget.AbsoluteLayout`/custom layout. Each platform's native
// container does the actual arrange — MPAPP just maps the attached bounds
// + flags to the native placement API.

#ifndef MPAPP_INTERNAL_BASIC_ABSOLUTE_LAYOUT_HPP
#define MPAPP_INTERNAL_BASIC_ABSOLUTE_LAYOUT_HPP

#include <cstdint>
#include <string_view>
#include <unordered_map>

#include "../layout.hpp"
#include "../observable.hpp"
#include "../platform.hpp"
#include "../view.hpp"

namespace mpapp {

// A child's placement rectangle inside an absolute_layout. Components are
// interpreted per the child's `absolute_layout_flags`: a proportional
// component is a fraction (0..1) of the container's corresponding extent;
// otherwise it is an absolute device-independent pixel value.
struct rect {
    double x      = 0.0;
    double y      = 0.0;
    double width  = 0.0;
    double height = 0.0;

    bool operator==(const rect&) const = default;
};

// Bitmask selecting which `rect` components are proportional. Mirrors
// MAUI's `AbsoluteLayoutFlags`. `position_proportional` == x|y,
// `size_proportional` == width|height, `all` == every component.
enum class absolute_layout_flags : std::uint8_t {
    none                  = 0,
    x_proportional        = 1,
    y_proportional        = 2,
    width_proportional    = 4,
    height_proportional   = 8,
    position_proportional = 3,   // x | y
    size_proportional     = 12,  // width | height
    all                   = 15,  // x | y | width | height
};

constexpr std::string_view to_string(absolute_layout_flags f) noexcept {
    switch (f) {
        case absolute_layout_flags::none:                  return "none";
        case absolute_layout_flags::x_proportional:        return "x_proportional";
        case absolute_layout_flags::y_proportional:        return "y_proportional";
        case absolute_layout_flags::width_proportional:    return "width_proportional";
        case absolute_layout_flags::height_proportional:   return "height_proportional";
        case absolute_layout_flags::position_proportional: return "position_proportional";
        case absolute_layout_flags::size_proportional:     return "size_proportional";
        case absolute_layout_flags::all:                   return "all";
    }
    return "?";
}

} // namespace mpapp

namespace mpapp::internal {

template <class Platform = platform::current>
class absolute_layout_handler;

class basic_absolute_layout : public layout {
public:
    basic_absolute_layout() = default;
    ~basic_absolute_layout() override = default;

    basic_absolute_layout(const basic_absolute_layout&)            = delete;
    basic_absolute_layout& operator=(const basic_absolute_layout&) = delete;
    basic_absolute_layout(basic_absolute_layout&&)                 = delete;
    basic_absolute_layout& operator=(basic_absolute_layout&&)      = delete;

    // ----- Per-child placement attached properties --------------------
    //
    // MAUI: AbsoluteLayout.SetLayoutBounds(child, rect);
    //       AbsoluteLayout.SetLayoutFlags(child, flags);
    // MPAPP: abs.set_layout_bounds(child, r); abs.set_layout_flags(child, f);
    //
    // Storage is a side map keyed on the child's view*. The layout's real
    // handler reads these in its add_child path to drive the native
    // placement API. Mirrors basic_grid_layout's cell-placement store.

    void set_layout_bounds(view& v, rect r)                    { attached_for(v).bounds = r; }
    void set_layout_flags(view& v, absolute_layout_flags f)    { attached_for(v).flags = f; }

    [[nodiscard]] rect get_layout_bounds(const view& v) const {
        auto it = attached_.find(const_cast<view*>(&v));
        return (it == attached_.end()) ? rect{} : it->second.bounds;
    }

    [[nodiscard]] absolute_layout_flags get_layout_flags(const view& v) const {
        auto it = attached_.find(const_cast<view*>(&v));
        return (it == attached_.end()) ? absolute_layout_flags::none : it->second.flags;
    }

    absolute_layout_handler<platform::current>&       handler() noexcept       { return *absolute_handler_; }
    const absolute_layout_handler<platform::current>& handler() const noexcept { return *absolute_handler_; }
    bool                                              has_handler() const noexcept { return absolute_handler_ != nullptr; }
    void                                              set_handler(absolute_layout_handler<platform::current>& h) noexcept { absolute_handler_ = &h; }

private:
    struct attached_state {
        rect                  bounds{};
        absolute_layout_flags flags = absolute_layout_flags::none;
    };

    attached_state& attached_for(view& v) {
        return attached_[&v];
    }

    absolute_layout_handler<platform::current>* absolute_handler_ = nullptr;
    // Attached-property store. Keyed on the child view*; cleared by the
    // application before the child's lifetime ends (mirrors the
    // basic_grid_layout placements_ store / page_stack attached store
    // pattern from ADR-0014).
    std::unordered_map<view*, attached_state> attached_{};
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_ABSOLUTE_LAYOUT_HPP
