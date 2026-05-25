// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Stepper.md
//
// `mpapp::stepper` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_stepper` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::stepper x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_stepper x;
//     mpapp::stepper_handler<mpapp::platform::mock> h;
//     h.map_value(x);

#ifndef MPAPP_STEPPER_HPP
#define MPAPP_STEPPER_HPP

#include "internal/basic_stepper.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_stepper` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/stepper_handler.hpp"

namespace mpapp {

class stepper : public internal::basic_stepper {
public:
    stepper() {
        set_handler(embedded_handler_);
        embedded_handler_.map_value(*this);
        embedded_handler_.map_minimum(*this);
        embedded_handler_.map_maximum(*this);
        embedded_handler_.map_interval(*this);
        embedded_handler_.map_gestures(*this);
    }

    stepper(const stepper&)            = delete;
    stepper& operator=(const stepper&) = delete;
    stepper(stepper&&)                 = delete;
    stepper& operator=(stepper&&)      = delete;

private:
    internal::stepper_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::stepper_handler<>` (host-current) and
// `mpapp::stepper_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using stepper_handler = internal::stepper_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_STEPPER_HPP
