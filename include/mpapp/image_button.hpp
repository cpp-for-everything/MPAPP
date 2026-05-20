// SPDX-License-Identifier: Apache-2.0
// `mpapp::image_button` — button whose content is an image. Click
// surface is deferred to M-05 polish — for now the widget displays the
// image and forwards layout but doesn't fire `clicked`. Use a plain
// `mpapp::button` wrapping a `mpapp::image` if clicks are required
// today.

#ifndef MPAPP_IMAGE_BUTTON_HPP
#define MPAPP_IMAGE_BUTTON_HPP

#include <string>

#include "image.hpp"      // aspect_mode
#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform>
class image_button_handler;

class image_button : public view {
public:
    image_button() = default;

    Observable<std::string>   source{};
    Observable<aspect_mode>   aspect{aspect_mode::aspect_fit};

    image_button_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const image_button_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                           has_handler() const noexcept { return handler_ != nullptr; }
    void                                           set_handler(image_button_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    image_button_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_IMAGE_BUTTON_HPP
