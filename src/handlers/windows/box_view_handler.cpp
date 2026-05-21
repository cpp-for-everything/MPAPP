// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 box_view handler implementation.

#include "mpapp/handlers/windows/box_view_handler.hpp"

#if defined(_WIN32)

#include <algorithm>
#include <cmath>
#include <cstdint>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.h>
#include <winrt/Microsoft.UI.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>

namespace mpapp {

namespace muxc  = ::winrt::Microsoft::UI::Xaml::Controls;
namespace muxm  = ::winrt::Microsoft::UI::Xaml::Media;
namespace muxui = ::winrt::Windows::UI;

namespace {

uint8_t to_byte(double v) {
    if (!(v == v)) return 0;                      // NaN
    if (v <= 0.0)  return 0;
    if (v >= 1.0)  return 255;
    return static_cast<uint8_t>(std::lround(v * 255.0));
}

muxui::Color to_wuc(const color& c) {
    muxui::Color out{};
    out.A = to_byte(c.a);
    out.R = to_byte(c.r);
    out.G = to_byte(c.g);
    out.B = to_byte(c.b);
    return out;
}

} // namespace

box_view_handler<platform::windows>::box_view_handler() {
    native_ = muxc::Border{};
    // MAUI BoxView default measured size is 40x40 dip.
    native_.Width(40.0);
    native_.Height(40.0);
}

box_view_handler<platform::windows>::~box_view_handler() = default;

void box_view_handler<platform::windows>::apply_fill(const color& c) {
    if (native_ == nullptr) return;
    muxm::SolidColorBrush brush{to_wuc(c)};
    native_.Background(brush);
}

void box_view_handler<platform::windows>::apply_corners(const corner_radius& r) {
    if (native_ == nullptr) return;
    ::winrt::Microsoft::UI::Xaml::CornerRadius cr{};
    cr.TopLeft     = r.top_left;
    cr.TopRight    = r.top_right;
    cr.BottomRight = r.bottom_right;
    cr.BottomLeft  = r.bottom_left;
    native_.CornerRadius(cr);
}

void box_view_handler<platform::windows>::map_fill(box_view& b) {
    bound_ = &b;
    apply_fill(b.fill.get());
    b.fill.changed.subscribe(fill_slot_, fill_cb_);
}

void box_view_handler<platform::windows>::map_corners(box_view& b) {
    apply_corners(b.corners.get());
    b.corners.changed.subscribe(corners_slot_, corners_cb_);
}

} // namespace mpapp


// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register box_view so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/box_view.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_box_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::box_view*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_box_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
