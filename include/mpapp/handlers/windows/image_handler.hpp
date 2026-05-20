// SPDX-License-Identifier: Apache-2.0
// WinUI 3 image handler — wraps `mux::Controls::Image` + `BitmapImage`.

#ifndef MPAPP_HANDLERS_WINDOWS_IMAGE_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_IMAGE_HANDLER_HPP

#include "../../image.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class image_handler<platform::windows> {
public:
    image_handler();
    ~image_handler();
    image_handler(const image_handler&)            = delete;
    image_handler& operator=(const image_handler&) = delete;

    void map_source(image& i);
    void map_aspect(image& i);

    winrt::Microsoft::UI::Xaml::Controls::Image&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Image& native() const noexcept { return native_; }

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

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_IMAGE_HANDLER_HPP
