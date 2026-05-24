// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Toolbar.md
//
// `mpapp::toolbar` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_toolbar` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::toolbar x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_toolbar x;
//     mpapp::toolbar_handler<mpapp::platform::mock> h;
//     h.map_items(x);

#ifndef MPAPP_TOOLBAR_HPP
#define MPAPP_TOOLBAR_HPP

#include "internal/basic_toolbar.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_toolbar` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/toolbar_handler.hpp"

namespace mpapp {

class toolbar : public internal::basic_toolbar {
public:
    toolbar() {
        set_handler(embedded_handler_);
        embedded_handler_.map_items(*this);
        embedded_handler_.map_title(*this);
    }

    toolbar(const toolbar&)            = delete;
    toolbar& operator=(const toolbar&) = delete;
    toolbar(toolbar&&)                 = delete;
    toolbar& operator=(toolbar&&)      = delete;

private:
    internal::toolbar_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::toolbar_handler<>` (host-current) and
// `mpapp::toolbar_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using toolbar_handler = internal::toolbar_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_TOOLBAR_HPP
