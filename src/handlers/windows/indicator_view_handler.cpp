// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 indicator_view handler implementation.

#include "mpapp/handlers/windows/indicator_view_handler.hpp"

#if defined(_WIN32)

#include <cstdint>
#include <cstdlib>
#include <string>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

namespace mpapp {

namespace mux   = ::winrt::Microsoft::UI::Xaml;
namespace muxc  = ::winrt::Microsoft::UI::Xaml::Controls;
namespace muxm  = ::winrt::Microsoft::UI::Xaml::Media;
namespace muxs  = ::winrt::Microsoft::UI::Xaml::Shapes;
namespace muxui = ::winrt::Windows::UI;

namespace {

constexpr double kDotSize    = 8.0;
constexpr double kDotSpacing = 4.0;

muxui::Color parse_color(const std::string& name, muxui::Color fallback) {
    if (name.empty()) return fallback;
    if (name[0] == '#') {
        uint32_t v = static_cast<uint32_t>(std::strtoul(name.c_str() + 1, nullptr, 16));
        muxui::Color out{};
        if (name.size() == 7) {
            out.A = 255;
            out.R = static_cast<uint8_t>((v >> 16) & 0xFF);
            out.G = static_cast<uint8_t>((v >>  8) & 0xFF);
            out.B = static_cast<uint8_t>( v        & 0xFF);
        } else if (name.size() == 9) {
            out.A = static_cast<uint8_t>((v >> 24) & 0xFF);
            out.R = static_cast<uint8_t>((v >> 16) & 0xFF);
            out.G = static_cast<uint8_t>((v >>  8) & 0xFF);
            out.B = static_cast<uint8_t>( v        & 0xFF);
        } else {
            return fallback;
        }
        return out;
    }
    if (name == "Red")   return {255, 220,  50,  50};
    if (name == "Green") return {255,  80, 180,  80};
    if (name == "Blue")  return {255,  60, 120, 220};
    if (name == "Black") return {255,   0,   0,   0};
    if (name == "White") return {255, 255, 255, 255};
    if (name == "Gray")  return {255, 160, 160, 160};
    if (name == "Teal")  return {255,   0, 150, 165};
    return fallback;
}

} // namespace

indicator_view_handler<platform::windows>::indicator_view_handler() {
    native_ = muxc::StackPanel{};
    native_.Orientation(muxc::Orientation::Horizontal);
    native_.Spacing(kDotSpacing);
}

indicator_view_handler<platform::windows>::~indicator_view_handler() = default;

void indicator_view_handler<platform::windows>::rebuild_dots() {
    if (native_ == nullptr) return;
    muxui::Color unsel = parse_color(cached_color_.name,    {255, 200, 200, 200});
    muxui::Color sel   = parse_color(cached_selected_.name, {255,  60, 120, 220});
    try {
        // Clear, then re-create dots with the correct fill color for each
        // index. This mirrors the Linux + Android handlers (which also
        // rebuild) and side-steps WinUI projection issues that arise when
        // indexing the IVector<UIElement> children inside a TU that doesn't
        // pull in the full IVector definition.
        native_.Children().Clear();
        for (int i = 0; i < cached_count_; ++i) {
            muxs::Ellipse e{};
            e.Width(kDotSize);
            e.Height(kDotSize);
            const muxui::Color c = (i == cached_position_) ? sel : unsel;
            e.Fill(muxm::SolidColorBrush{c});
            native_.Children().Append(e);
        }
    } catch (...) {}
}

void indicator_view_handler<platform::windows>::recolor_dots() {
    // Cheapest correct path: rebuild — same trade-off as Linux/Android.
    rebuild_dots();
}

void indicator_view_handler<platform::windows>::apply_count(int v) {
    if (v < 0) v = 0;
    cached_count_ = v;
    rebuild_dots();
}

void indicator_view_handler<platform::windows>::apply_position(int v) {
    cached_position_ = v;
    recolor_dots();
}

void indicator_view_handler<platform::windows>::apply_indicator_color(const brush_ref& b) {
    cached_color_ = b;
    recolor_dots();
}

void indicator_view_handler<platform::windows>::apply_selected_indicator_color(const brush_ref& b) {
    cached_selected_ = b;
    recolor_dots();
}

void indicator_view_handler<platform::windows>::map_count(indicator_view& iv) {
    bound_ = &iv;
    apply_count(iv.count.get());
    iv.count.changed.subscribe(count_slot_, count_cb_);
}
void indicator_view_handler<platform::windows>::map_position(indicator_view& iv) {
    bound_ = &iv;
    apply_position(iv.position.get());
    iv.position.changed.subscribe(position_slot_, position_cb_);
}
void indicator_view_handler<platform::windows>::map_indicator_color(indicator_view& iv) {
    bound_ = &iv;
    apply_indicator_color(iv.indicator_color.get());
    iv.indicator_color.changed.subscribe(color_slot_, color_cb_);
}
void indicator_view_handler<platform::windows>::map_selected_indicator_color(indicator_view& iv) {
    bound_ = &iv;
    apply_selected_indicator_color(iv.selected_indicator_color.get());
    iv.selected_indicator_color.changed.subscribe(sel_color_slot_, sel_color_cb_);
}

} // namespace mpapp

// ----- ADR-0013 self-registration --------------------------------------------

namespace {
::winrt::Microsoft::UI::Xaml::UIElement dispatch_indicator_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::indicator_view*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}
struct registrar {
    registrar() {
        ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_indicator_view);
    }
};
[[maybe_unused]] registrar _reg;
} // namespace

#endif // _WIN32
