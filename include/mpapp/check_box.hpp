// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/CheckBox.md
//
// `mpapp::check_box` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_check_box` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::check_box x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_check_box x;
//     mpapp::check_box_handler<mpapp::platform::mock> h;
//     h.map_is_checked(x);

#ifndef MPAPP_CHECK_BOX_HPP
#define MPAPP_CHECK_BOX_HPP

#include "internal/basic_check_box.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_check_box` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/check_box_handler.hpp"

namespace mpapp {

class check_box : public internal::basic_check_box {
public:
    check_box() {
        set_handler(embedded_handler_);
        embedded_handler_.map_is_checked(*this);
        embedded_handler_.map_gestures(*this);
    }

    check_box(const check_box&)            = delete;
    check_box& operator=(const check_box&) = delete;
    check_box(check_box&&)                 = delete;
    check_box& operator=(check_box&&)      = delete;

private:
    internal::check_box_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::check_box_handler<>` (host-current) and
// `mpapp::check_box_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using check_box_handler = internal::check_box_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_CHECK_BOX_HPP
