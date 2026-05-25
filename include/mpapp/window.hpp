// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Window.md
//
// `mpapp::window` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_window` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::window x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_window x;
//     mpapp::window_handler<mpapp::platform::mock> h;
//     h.map_text(x);

#ifndef MPAPP_WINDOW_HPP
#define MPAPP_WINDOW_HPP

#include "internal/basic_window.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_window` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/window_handler.hpp"

namespace mpapp {

class window : public internal::basic_window {
public:
    window() {
        set_handler(embedded_handler_);
        embedded_handler_.bind(*this);
        embedded_handler_.map_gestures(*this);
    }

    window(const window&)            = delete;
    window& operator=(const window&) = delete;
    window(window&&)                 = delete;
    window& operator=(window&&)      = delete;

private:
    internal::window_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::window_handler<>` (host-current) and
// `mpapp::window_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using window_handler = internal::window_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_WINDOW_HPP
