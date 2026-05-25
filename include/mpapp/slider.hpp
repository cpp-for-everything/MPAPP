// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Slider.md
//
// `mpapp::slider` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_slider` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::slider x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_slider x;
//     mpapp::slider_handler<mpapp::platform::mock> h;
//     h.map_value(x);

#ifndef MPAPP_SLIDER_HPP
#define MPAPP_SLIDER_HPP

#include "internal/basic_slider.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_slider` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/slider_handler.hpp"

namespace mpapp {

class slider : public internal::basic_slider {
public:
    slider() {
        set_handler(embedded_handler_);
        embedded_handler_.map_value(*this);
        embedded_handler_.map_minimum(*this);
        embedded_handler_.map_maximum(*this);
        embedded_handler_.map_gestures(*this);
    }

    slider(const slider&)            = delete;
    slider& operator=(const slider&) = delete;
    slider(slider&&)                 = delete;
    slider& operator=(slider&&)      = delete;

private:
    internal::slider_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::slider_handler<>` (host-current) and
// `mpapp::slider_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using slider_handler = internal::slider_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_SLIDER_HPP
