// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/TemplatedView.md
//
// `mpapp::templated_view` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_templated_view` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::templated_view x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_templated_view x;
//     mpapp::templated_view_handler<mpapp::platform::mock> h;
//     h.map_content(x);

#ifndef MPAPP_TEMPLATED_VIEW_HPP
#define MPAPP_TEMPLATED_VIEW_HPP

#include "internal/basic_templated_view.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_templated_view` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/templated_view_handler.hpp"

namespace mpapp {

class templated_view : public internal::basic_templated_view {
public:
    templated_view() {
        set_handler(embedded_handler_);
        embedded_handler_.map_content(*this);
        embedded_handler_.map_template_id(*this);
        embedded_handler_.map_gestures(*this);
    }

    templated_view(const templated_view&)            = delete;
    templated_view& operator=(const templated_view&) = delete;
    templated_view(templated_view&&)                 = delete;
    templated_view& operator=(templated_view&&)      = delete;

private:
    internal::templated_view_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::templated_view_handler<>` (host-current) and
// `mpapp::templated_view_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using templated_view_handler = internal::templated_view_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_TEMPLATED_VIEW_HPP
