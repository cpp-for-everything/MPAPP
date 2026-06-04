// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_absolute_layout handler implementation. mux::Controls::Canvas
// is the native analogue to MAUI's AbsoluteLayout: children are placed via
// Canvas.SetLeft/SetTop with explicit Width/Height. Proportional layout_flags
// are resolved against the Canvas's current ActualWidth/ActualHeight.

#include "mpapp/handlers/windows/absolute_layout_handler.hpp"

#if defined(_WIN32)

#include <cstdint>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

namespace mpapp::internal {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

absolute_layout_handler<platform::windows>::absolute_layout_handler() {
    native_ = muxc::Canvas{};
}

absolute_layout_handler<platform::windows>::~absolute_layout_handler() = default;

void absolute_layout_handler<platform::windows>::apply_bounds(
    view& child, const rect& r, absolute_layout_flags f) {
    if (native_ == nullptr) {
        return;
    }
    auto el = detail::windows_dispatch::dispatch(&child);
    if (el == nullptr) {
        return;
    }

    // Proportional components are a 0..1 fraction of the Canvas extent;
    // absolute components are device-independent pixels.
    const double cw  = native_.ActualWidth();
    const double ch  = native_.ActualHeight();
    const auto   bit = static_cast<std::uint8_t>(f);
    const auto   is_prop = [bit](absolute_layout_flags m) {
        return (bit & static_cast<std::uint8_t>(m)) != 0;
    };

    const double x = is_prop(absolute_layout_flags::x_proportional)      ? r.x * cw     : r.x;
    const double y = is_prop(absolute_layout_flags::y_proportional)      ? r.y * ch     : r.y;
    const double w = is_prop(absolute_layout_flags::width_proportional)  ? r.width * cw : r.width;
    const double h = is_prop(absolute_layout_flags::height_proportional) ? r.height * ch: r.height;

    muxc::Canvas::SetLeft(el, x);
    muxc::Canvas::SetTop(el, y);
    if (auto fe = el.try_as<mux::FrameworkElement>()) {
        if (w > 0.0) { fe.Width(w); }
        if (h > 0.0) { fe.Height(h); }
    }
}

void absolute_layout_handler<platform::windows>::map_layout_bounds(
    basic_absolute_layout& a, view& child) {
    apply_bounds(child, a.get_layout_bounds(child), a.get_layout_flags(child));
}

void absolute_layout_handler<platform::windows>::map_layout_flags(
    basic_absolute_layout& a, view& child) {
    apply_bounds(child, a.get_layout_bounds(child), a.get_layout_flags(child));
}

void absolute_layout_handler<platform::windows>::add_child(
    basic_absolute_layout& a, view& child) {
    if (native_ == nullptr) {
        return;
    }
    auto el = detail::windows_dispatch::dispatch(&child);
    if (el == nullptr) {
        return;
    }
    native_.Children().Append(el);
    apply_bounds(child, a.get_layout_bounds(child), a.get_layout_flags(child));
}

} // namespace mpapp::internal

#endif // _WIN32
