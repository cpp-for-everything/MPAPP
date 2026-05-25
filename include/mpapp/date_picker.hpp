// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/DatePicker.md
//
// `mpapp::date_picker` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_date_picker` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::date_picker x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_date_picker x;
//     mpapp::date_picker_handler<mpapp::platform::mock> h;
//     h.map_date(x);

#ifndef MPAPP_DATE_PICKER_HPP
#define MPAPP_DATE_PICKER_HPP

#include "internal/basic_date_picker.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_date_picker` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/date_picker_handler.hpp"

namespace mpapp {

class date_picker : public internal::basic_date_picker {
public:
    date_picker() {
        set_handler(embedded_handler_);
        embedded_handler_.map_date(*this);
        embedded_handler_.map_gestures(*this);
    }

    date_picker(const date_picker&)            = delete;
    date_picker& operator=(const date_picker&) = delete;
    date_picker(date_picker&&)                 = delete;
    date_picker& operator=(date_picker&&)      = delete;

private:
    internal::date_picker_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::date_picker_handler<>` (host-current) and
// `mpapp::date_picker_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using date_picker_handler = internal::date_picker_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_DATE_PICKER_HPP
