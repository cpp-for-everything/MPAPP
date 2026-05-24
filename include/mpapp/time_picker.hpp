// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/TimePicker.md
//
// `mpapp::time_picker` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_time_picker` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::time_picker x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_time_picker x;
//     mpapp::time_picker_handler<mpapp::platform::mock> h;
//     h.map_time(x);

#ifndef MPAPP_TIME_PICKER_HPP
#define MPAPP_TIME_PICKER_HPP

#include "internal/basic_time_picker.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_time_picker` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/time_picker_handler.hpp"

namespace mpapp {

class time_picker : public internal::basic_time_picker {
public:
    time_picker() {
        set_handler(embedded_handler_);
        embedded_handler_.map_time(*this);
    }

    time_picker(const time_picker&)            = delete;
    time_picker& operator=(const time_picker&) = delete;
    time_picker(time_picker&&)                 = delete;
    time_picker& operator=(time_picker&&)      = delete;

private:
    internal::time_picker_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::time_picker_handler<>` (host-current) and
// `mpapp::time_picker_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using time_picker_handler = internal::time_picker_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_TIME_PICKER_HPP
