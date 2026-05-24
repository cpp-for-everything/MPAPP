// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Entry.md
//
// `mpapp::entry` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_entry` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::entry x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_entry x;
//     mpapp::entry_handler<mpapp::platform::mock> h;
//     h.map_text(x);

#ifndef MPAPP_ENTRY_HPP
#define MPAPP_ENTRY_HPP

#include "internal/basic_entry.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_entry` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/entry_handler.hpp"

namespace mpapp {

class entry : public internal::basic_entry {
public:
    entry() {
        set_handler(embedded_handler_);
        embedded_handler_.map_text(*this);
        embedded_handler_.map_placeholder(*this);
        embedded_handler_.map_is_read_only(*this);
    }

    entry(const entry&)            = delete;
    entry& operator=(const entry&) = delete;
    entry(entry&&)                 = delete;
    entry& operator=(entry&&)      = delete;

private:
    internal::entry_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::entry_handler<>` (host-current) and
// `mpapp::entry_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using entry_handler = internal::entry_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_ENTRY_HPP
