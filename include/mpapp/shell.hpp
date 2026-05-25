// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Shell.md
//
// `mpapp::shell` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_shell` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::shell x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_shell x;
//     mpapp::shell_handler<mpapp::platform::mock> h;
//     h.map_tabs(x);

#ifndef MPAPP_SHELL_HPP
#define MPAPP_SHELL_HPP

#include "internal/basic_shell.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_shell` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/shell_handler.hpp"

namespace mpapp {

class shell : public internal::basic_shell {
public:
    shell() {
        set_shell_handler(embedded_handler_);
        embedded_handler_.map_tabs(*this);
        embedded_handler_.map_current_tab_index(*this);
        embedded_handler_.map_is_flyout_open(*this);
        embedded_handler_.map_flyout_content(*this);
        embedded_handler_.map_current_content(*this);
        embedded_handler_.map_gestures(*this);
    }

    shell(const shell&)            = delete;
    shell& operator=(const shell&) = delete;
    shell(shell&&)                 = delete;
    shell& operator=(shell&&)      = delete;

private:
    internal::shell_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::shell_handler<>` (host-current) and
// `mpapp::shell_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using shell_handler = internal::shell_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_SHELL_HPP
