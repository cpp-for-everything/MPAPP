// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/AbsoluteLayout.md
//
// `mpapp::absolute_layout` — user-facing wrapper around the
// platform-agnostic `mpapp::internal::basic_absolute_layout` surface.
// Embeds the per-platform handler by value and auto-binds it in the
// constructor, so app code reads as `mpapp::absolute_layout x;
// x.set_layout_bounds(child, {...});` with no separate handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_absolute_layout x;
//     mpapp::absolute_layout_handler<mpapp::platform::mock> h;
//     h.map_layout_bounds(x, child);

#ifndef MPAPP_ABSOLUTE_LAYOUT_HPP
#define MPAPP_ABSOLUTE_LAYOUT_HPP

#include "internal/basic_absolute_layout.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_absolute_layout` as a complete type now, which lets its inline
// bodies (mock + per-platform) access surface members.
#include "handlers/absolute_layout_handler.hpp"

namespace mpapp {

class absolute_layout : public internal::basic_absolute_layout {
public:
    absolute_layout() {
        set_handler(embedded_handler_);
        embedded_handler_.map_gestures(*this);
    }

    absolute_layout(const absolute_layout&)            = delete;
    absolute_layout& operator=(const absolute_layout&) = delete;
    absolute_layout(absolute_layout&&)                 = delete;
    absolute_layout& operator=(absolute_layout&&)      = delete;

private:
    internal::absolute_layout_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::absolute_layout_handler<>` (host-current) and
// `mpapp::absolute_layout_handler<platform::mock>` both work without
// naming `internal::`.
template <class Platform = platform::current>
using absolute_layout_handler = internal::absolute_layout_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_ABSOLUTE_LAYOUT_HPP
