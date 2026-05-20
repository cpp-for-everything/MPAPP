// SPDX-License-Identifier: Apache-2.0
// WinUI 3 image_button handler — `mux::Controls::Button` whose content
// is a `mux::Controls::Image`. Click event deferred to M-05 polish.

#ifndef MPAPP_HANDLERS_WINDOWS_IMAGE_BUTTON_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_IMAGE_BUTTON_HANDLER_HPP

#include "../../image_button.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class image_button_handler<platform::windows> {
public:
    image_button_handler();
    ~image_button_handler();
    image_button_handler(const image_button_handler&)            = delete;
    image_button_handler& operator=(const image_button_handler&) = delete;

    void map_source(image_button& b);
    void map_aspect(image_button& b);

    winrt::Microsoft::UI::Xaml::Controls::Button&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Button& native() const noexcept { return native_; }

private:
    void apply_source(const std::string& v);
    void apply_aspect(aspect_mode v);

    struct source_cb_t { image_button_handler<platform::windows>* self; void operator()(const std::string& v) const { self->apply_source(v); } };
    struct aspect_cb_t { image_button_handler<platform::windows>* self; void operator()(aspect_mode v) const { self->apply_aspect(v); } };

    winrt::Microsoft::UI::Xaml::Controls::Button native_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::Image  image_{nullptr};

    source_cb_t                        source_cb_{this};
    aspect_cb_t                        aspect_cb_{this};
    signal_slot<const std::string&>    source_slot_{};
    signal_slot<const aspect_mode&>    aspect_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_IMAGE_BUTTON_HANDLER_HPP
