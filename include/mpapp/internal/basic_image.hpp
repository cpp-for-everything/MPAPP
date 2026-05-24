// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. `mpapp::image` — file-backed image display.
// Bitmap source is a file path on the local FS; URL / resource-id /
// stream sources are deferred to M-05 polish.

#ifndef MPAPP_INTERNAL_BASIC_IMAGE_HPP
#define MPAPP_INTERNAL_BASIC_IMAGE_HPP

#include <cstdint>
#include <string>

#include "../observable.hpp"
#include "../platform.hpp"
#include "../view.hpp"

namespace mpapp {

enum class aspect_mode : std::uint8_t {
    aspect_fit  = 0,   // letterboxed, keeping ratio
    aspect_fill = 1,   // crop to fill
    fill        = 2,   // stretch, ignore ratio
    center      = 3,   // no scale; center
};

} // namespace mpapp

namespace mpapp::internal {

template <class Platform = platform::current>
class image_handler;

class basic_image : public view {
public:
    basic_image() = default;

    Observable<std::string>   source{};
    Observable<aspect_mode>   aspect{aspect_mode::aspect_fit};

    image_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const image_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                    has_handler() const noexcept { return handler_ != nullptr; }
    void                                    set_handler(image_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    image_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_IMAGE_HPP
