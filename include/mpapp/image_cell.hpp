// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ImageCell.md
//
// `mpapp::image_cell` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_image_cell` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::image_cell x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_image_cell x;
//     mpapp::image_cell_handler<mpapp::platform::mock> h;
//     h.map_text(x);

#ifndef MPAPP_IMAGE_CELL_HPP
#define MPAPP_IMAGE_CELL_HPP

#include "internal/basic_image_cell.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_image_cell` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/image_cell_handler.hpp"

namespace mpapp {

class image_cell : public internal::basic_image_cell {
public:
    image_cell() {
        set_ic_handler(embedded_handler_);
        embedded_handler_.map_text(*this);
        embedded_handler_.map_detail(*this);
        embedded_handler_.map_image_uri(*this);
        embedded_handler_.map_gestures(*this);
    }

    image_cell(const image_cell&)            = delete;
    image_cell& operator=(const image_cell&) = delete;
    image_cell(image_cell&&)                 = delete;
    image_cell& operator=(image_cell&&)      = delete;

private:
    internal::image_cell_handler<platform::current> embedded_handler_;
};

// Template alias so `mpapp::image_cell_handler<>` (host-current) and
// `mpapp::image_cell_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using image_cell_handler = internal::image_cell_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_IMAGE_CELL_HPP
