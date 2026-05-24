// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Switch.md
//
// `mpapp::switch_` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_switch_` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::switch_ x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_switch_ x;
//     mpapp::switch_handler<mpapp::platform::mock> h;
//     h.map_is_on(x);

#ifndef MPAPP_SWITCH__HPP
#define MPAPP_SWITCH__HPP

#include "internal/basic_switch_.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_switch_` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/switch_handler.hpp"

namespace mpapp {

class switch_ : public internal::basic_switch_ {
public:
    switch_() {
        set_handler(embedded_handler_);
        embedded_handler_.map_is_on(*this);
    }

    switch_(const switch_&)            = delete;
    switch_& operator=(const switch_&) = delete;
    switch_(switch_&&)                 = delete;
    switch_& operator=(switch_&&)      = delete;

private:
    internal::switch_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::switch_handler<>` (host-current) and
// `mpapp::switch_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using switch_handler = internal::switch_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_SWITCH__HPP
