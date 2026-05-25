// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Image.md
//
// `mpapp::image` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_image` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::image x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_image x;
//     mpapp::image_handler<mpapp::platform::mock> h;
//     h.map_source(x);

#ifndef MPAPP_IMAGE_HPP
#define MPAPP_IMAGE_HPP

#include "internal/basic_image.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_image` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/image_handler.hpp"

namespace mpapp {

class image : public internal::basic_image {
public:
    image() {
        set_handler(embedded_handler_);
        embedded_handler_.map_source(*this);
        embedded_handler_.map_aspect(*this);
        embedded_handler_.map_gestures(*this);
    }

    image(const image&)            = delete;
    image& operator=(const image&) = delete;
    image(image&&)                 = delete;
    image& operator=(image&&)      = delete;

private:
    internal::image_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::image_handler<>` (host-current) and
// `mpapp::image_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using image_handler = internal::image_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_IMAGE_HPP
