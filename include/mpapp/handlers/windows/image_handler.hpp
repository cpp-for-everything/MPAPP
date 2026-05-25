// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_image handler — wraps `mux::Controls::Image` + `BitmapImage`.

#ifndef MPAPP_HANDLERS_WINDOWS_IMAGE_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_IMAGE_HANDLER_HPP

#include "../../internal/basic_image.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class image_handler<platform::windows> {
public:
    image_handler();
    ~image_handler();
    image_handler(const image_handler&)            = delete;
    image_handler& operator=(const image_handler&) = delete;

    void map_source(basic_image& i);
    void map_aspect(basic_image& i);

    winrt::Microsoft::UI::Xaml::Controls::Image&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Image& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_image& /*x*/) noexcept {}


private:
    void apply_source(const std::string& v);
    void apply_aspect(aspect_mode v);

    struct source_cb_t { image_handler<platform::windows>* self; void operator()(const std::string& v) const { self->apply_source(v); } };
    struct aspect_cb_t { image_handler<platform::windows>* self; void operator()(aspect_mode v) const { self->apply_aspect(v); } };

    winrt::Microsoft::UI::Xaml::Controls::Image native_{nullptr};

    source_cb_t                        source_cb_{this};
    aspect_cb_t                        aspect_cb_{this};
    signal_slot<const std::string&>    source_slot_{};
    signal_slot<const aspect_mode&>    aspect_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_IMAGE_HANDLER_HPP
