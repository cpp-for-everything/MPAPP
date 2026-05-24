// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ProgressBar.md
//
// `mpapp::progress_bar` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_progress_bar` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::progress_bar x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_progress_bar x;
//     mpapp::progress_bar_handler<mpapp::platform::mock> h;
//     h.map_progress(x);

#ifndef MPAPP_PROGRESS_BAR_HPP
#define MPAPP_PROGRESS_BAR_HPP

#include "internal/basic_progress_bar.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_progress_bar` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/progress_bar_handler.hpp"

namespace mpapp {

class progress_bar : public internal::basic_progress_bar {
public:
    progress_bar() {
        set_handler(embedded_handler_);
        embedded_handler_.map_progress(*this);
        embedded_handler_.map_color(*this);
        embedded_handler_.map_background_color(*this);
    }

    progress_bar(const progress_bar&)            = delete;
    progress_bar& operator=(const progress_bar&) = delete;
    progress_bar(progress_bar&&)                 = delete;
    progress_bar& operator=(progress_bar&&)      = delete;

private:
    internal::progress_bar_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::progress_bar_handler<>` (host-current) and
// `mpapp::progress_bar_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using progress_bar_handler = internal::progress_bar_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_PROGRESS_BAR_HPP
