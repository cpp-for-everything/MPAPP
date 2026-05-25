// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/RadioButton.md
//
// `mpapp::radio_button` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_radio_button` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::radio_button x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_radio_button x;
//     mpapp::radio_button_handler<mpapp::platform::mock> h;
//     h.map_is_checked(x);

#ifndef MPAPP_RADIO_BUTTON_HPP
#define MPAPP_RADIO_BUTTON_HPP

#include "internal/basic_radio_button.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_radio_button` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/radio_button_handler.hpp"

namespace mpapp {

class radio_button : public internal::basic_radio_button {
public:
    radio_button() {
        set_handler(embedded_handler_);
        embedded_handler_.map_is_checked(*this);
        embedded_handler_.map_group_name(*this);
        embedded_handler_.map_gestures(*this);
    }

    radio_button(const radio_button&)            = delete;
    radio_button& operator=(const radio_button&) = delete;
    radio_button(radio_button&&)                 = delete;
    radio_button& operator=(radio_button&&)      = delete;

private:
    internal::radio_button_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::radio_button_handler<>` (host-current) and
// `mpapp::radio_button_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using radio_button_handler = internal::radio_button_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_RADIO_BUTTON_HPP
