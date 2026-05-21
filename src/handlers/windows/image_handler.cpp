// SPDX-License-Identifier: Apache-2.0
// WinUI 3 image handler implementation.

#include "mpapp/handlers/windows/image_handler.hpp"

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

image_handler<platform::windows>::image_handler() {
    native_ = muxc::Image{};
}

image_handler<platform::windows>::~image_handler() = default;

void image_handler<platform::windows>::apply_source(const std::string& v) {
    if (native_ == nullptr) return;
    try {
        if (v.empty()) {
            native_.Source(nullptr);
            return;
        }
        // Local file path → file:// URI.
        std::string uri_str = (v.starts_with("http://") || v.starts_with("https://") || v.starts_with("file:") || v.starts_with("ms-appx:"))
            ? v
            : (std::string("file:///") + v);
        // Replace backslashes with forward slashes for file:// URIs.
        for (auto& c : uri_str) if (c == '\\') c = '/';
        wf::Uri uri{detail::to_hstring_utf8(uri_str)};
        muxmi::BitmapImage bmp{uri};
        native_.Source(bmp);
    } catch (...) {}
}

void image_handler<platform::windows>::apply_aspect(aspect_mode v) {
    if (native_ == nullptr) return;
    using muxm::Stretch;
    switch (v) {
        case aspect_mode::aspect_fit:  native_.Stretch(Stretch::Uniform);     break;
        case aspect_mode::aspect_fill: native_.Stretch(Stretch::UniformToFill); break;
        case aspect_mode::fill:        native_.Stretch(Stretch::Fill);        break;
        case aspect_mode::center:      native_.Stretch(Stretch::None);        break;
    }
}

void image_handler<platform::windows>::map_source(image& i) {
    apply_source(i.source.get());
    i.source.changed.subscribe(source_slot_, source_cb_);
}
void image_handler<platform::windows>::map_aspect(image& i) {
    apply_aspect(i.aspect.get());
    i.aspect.changed.subscribe(aspect_slot_, aspect_cb_);
}

} // namespace mpapp


// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register image so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/image.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_image(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::image*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_image); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
