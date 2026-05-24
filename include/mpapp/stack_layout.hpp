// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/StackLayout.md
//
// `mpapp::stack_layout` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_stack_layout` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::stack_layout x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_stack_layout x;
//     mpapp::stack_layout_handler<mpapp::platform::mock> h;
//     h.map_text(x);

#ifndef MPAPP_STACK_LAYOUT_HPP
#define MPAPP_STACK_LAYOUT_HPP

#include "internal/basic_stack_layout.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_stack_layout` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/stack_layout_handler.hpp"

namespace mpapp {

class stack_layout : public internal::basic_stack_layout {
public:
    stack_layout() {
        set_handler(embedded_handler_);
        embedded_handler_.bind(*this);
    }

    stack_layout(const stack_layout&)            = delete;
    stack_layout& operator=(const stack_layout&) = delete;
    stack_layout(stack_layout&&)                 = delete;
    stack_layout& operator=(stack_layout&&)      = delete;

private:
    internal::stack_layout_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::stack_layout_handler<>` (host-current) and
// `mpapp::stack_layout_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using stack_layout_handler = internal::stack_layout_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_STACK_LAYOUT_HPP
