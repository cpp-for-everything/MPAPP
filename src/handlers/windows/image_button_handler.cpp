// SPDX-License-Identifier: Apache-2.0
// WinUI 3 image_button handler implementation.

#include "mpapp/handlers/windows/image_button_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>

#include "winrt_strings.hpp"

namespace mpapp {

namespace muxc  = ::winrt::Microsoft::UI::Xaml::Controls;
namespace muxmi = ::winrt::Microsoft::UI::Xaml::Media::Imaging;
namespace muxm  = ::winrt::Microsoft::UI::Xaml::Media;
namespace wf    = ::winrt::Windows::Foundation;

image_button_handler<platform::windows>::image_button_handler() {
    native_ = muxc::Button{};
    image_  = muxc::Image{};
    native_.Content(image_);
}

image_button_handler<platform::windows>::~image_button_handler() = default;

void image_button_handler<platform::windows>::apply_source(const std::string& v) {
    if (image_ == nullptr) return;
    try {
        if (v.empty()) { image_.Source(nullptr); return; }
        std::string uri_str = (v.starts_with("http://") || v.starts_with("https://") || v.starts_with("file:") || v.starts_with("ms-appx:"))
            ? v
            : (std::string("file:///") + v);
        for (auto& c : uri_str) if (c == '\\') c = '/';
        wf::Uri uri{detail::to_hstring_utf8(uri_str)};
        muxmi::BitmapImage bmp{uri};
        image_.Source(bmp);
    } catch (...) {}
}

void image_button_handler<platform::windows>::apply_aspect(aspect_mode v) {
    if (image_ == nullptr) return;
    using muxm::Stretch;
    switch (v) {
        case aspect_mode::aspect_fit:  image_.Stretch(Stretch::Uniform);       break;
        case aspect_mode::aspect_fill: image_.Stretch(Stretch::UniformToFill); break;
        case aspect_mode::fill:        image_.Stretch(Stretch::Fill);          break;
        case aspect_mode::center:      image_.Stretch(Stretch::None);          break;
    }
}

void image_button_handler<platform::windows>::map_source(image_button& b) {
    apply_source(b.source.get());
    b.source.changed.subscribe(source_slot_, source_cb_);
}
void image_button_handler<platform::windows>::map_aspect(image_button& b) {
    apply_aspect(b.aspect.get());
    b.aspect.changed.subscribe(aspect_slot_, aspect_cb_);
}

} // namespace mpapp

#endif // _WIN32
