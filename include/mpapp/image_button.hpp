// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ImageButton.md
//
// `mpapp::image_button` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_image_button` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::image_button x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_image_button x;
//     mpapp::image_button_handler<mpapp::platform::mock> h;
//     h.map_source(x);

#ifndef MPAPP_IMAGE_BUTTON_HPP
#define MPAPP_IMAGE_BUTTON_HPP

#include "internal/basic_image_button.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_image_button` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/image_button_handler.hpp"

namespace mpapp {

class image_button : public internal::basic_image_button {
public:
    image_button() {
        set_handler(embedded_handler_);
        embedded_handler_.map_source(*this);
        embedded_handler_.map_aspect(*this);
        embedded_handler_.map_gestures(*this);
    }

    image_button(const image_button&)            = delete;
    image_button& operator=(const image_button&) = delete;
    image_button(image_button&&)                 = delete;
    image_button& operator=(image_button&&)      = delete;

private:
    internal::image_button_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::image_button_handler<>` (host-current) and
// `mpapp::image_button_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using image_button_handler = internal::image_button_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_IMAGE_BUTTON_HPP
