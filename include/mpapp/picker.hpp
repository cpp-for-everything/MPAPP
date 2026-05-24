// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Picker.md
//
// `mpapp::picker` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_picker` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::picker x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_picker x;
//     mpapp::picker_handler<mpapp::platform::mock> h;
//     h.map_items(x);

#ifndef MPAPP_PICKER_HPP
#define MPAPP_PICKER_HPP

#include "internal/basic_picker.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_picker` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/picker_handler.hpp"

namespace mpapp {

class picker : public internal::basic_picker {
public:
    picker() {
        set_handler(embedded_handler_);
        embedded_handler_.map_items(*this);
        embedded_handler_.map_selected_index(*this);
        embedded_handler_.map_title(*this);
    }

    picker(const picker&)            = delete;
    picker& operator=(const picker&) = delete;
    picker(picker&&)                 = delete;
    picker& operator=(picker&&)      = delete;

private:
    internal::picker_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::picker_handler<>` (host-current) and
// `mpapp::picker_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using picker_handler = internal::picker_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_PICKER_HPP
