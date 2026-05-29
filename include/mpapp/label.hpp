// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Label.md
//
// `mpapp::label` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_label` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::label x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_label x;
//     mpapp::label_handler<mpapp::platform::mock> h;
//     h.map_text(x);

#ifndef MPAPP_LABEL_HPP
#define MPAPP_LABEL_HPP

#include "internal/basic_label.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_label` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/label_handler.hpp"

namespace mpapp {

class label : public internal::basic_label {
public:
    label() {
        set_handler(embedded_handler_);
        embedded_handler_.map_text(*this);
        embedded_handler_.map_font_size(*this);
        embedded_handler_.map_font_bold(*this);
        embedded_handler_.map_font_family(*this);
        embedded_handler_.map_text_color(*this);
        embedded_handler_.map_gestures(*this);
    }

    label(const label&)            = delete;
    label& operator=(const label&) = delete;
    label(label&&)                 = delete;
    label& operator=(label&&)      = delete;

private:
    internal::label_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::label_handler<>` (host-current) and
// `mpapp::label_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using label_handler = internal::label_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_LABEL_HPP
