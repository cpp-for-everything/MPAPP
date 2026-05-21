// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 frame handler implementation. `mpapp::frame` is
// the deprecated MAUI-9 alias for `Border` — same native control, same
// surface, kept for one-to-one XAML migration parity.

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#endif

#include "mpapp/handlers/windows/frame_handler.hpp"

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

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/frame.hpp"
#include "mpapp/view.hpp"

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

frame_handler<platform::windows>::frame_handler() {
    native_ = muxc::Border{};
    // MAUI defaults: 1px border, padding 20.
    native_.BorderThickness({1.0, 1.0, 1.0, 1.0});
    native_.Padding({20.0, 20.0, 20.0, 20.0});
}

frame_handler<platform::windows>::~frame_handler() = default;

void frame_handler<platform::windows>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    view* raw = v.get();
    if (raw == nullptr) { native_.Child(nullptr); return; }
    if (auto el = detail::windows_dispatch::dispatch(raw); el != nullptr) {
        native_.Child(el);
    }
}

void frame_handler<platform::windows>::apply_border_color(const color& c) {
    if (native_ == nullptr) return;
    muxm::SolidColorBrush brush{to_wuc(c)};
    native_.BorderBrush(brush);
}

void frame_handler<platform::windows>::apply_has_shadow(bool /*b*/) {
    // MAUI's Windows Frame renderer historically ignores HasShadow; mirror
    // that for parity. The flag is recorded by the binding so XAML still
    // round-trips correctly; the native control just doesn't visualise it.
}

void frame_handler<platform::windows>::apply_corner_radius(float r) {
    if (native_ == nullptr) return;
    // -1 means "platform default" → 0 corners on Windows (WinUI Border
    // has no system default radius).
    const double v = (r < 0.0f) ? 0.0 : static_cast<double>(r);
    ::winrt::Microsoft::UI::Xaml::CornerRadius cr{};
    cr.TopLeft     = v;
    cr.TopRight    = v;
    cr.BottomRight = v;
    cr.BottomLeft  = v;
    native_.CornerRadius(cr);
}

void frame_handler<platform::windows>::apply_padding(const thickness& t) {
    if (native_ == nullptr) return;
    ::winrt::Microsoft::UI::Xaml::Thickness wt{t.left, t.top, t.right, t.bottom};
    native_.Padding(wt);
}

void frame_handler<platform::windows>::map_content(frame& f) {
    apply_content(f.content.get());
    f.content.changed.subscribe(content_slot_, content_cb_);
}

void frame_handler<platform::windows>::map_border_color(frame& f) {
    apply_border_color(f.border_color.get());
    f.border_color.changed.subscribe(border_color_slot_, border_color_cb_);
}

void frame_handler<platform::windows>::map_has_shadow(frame& f) {
    apply_has_shadow(f.has_shadow.get());
    f.has_shadow.changed.subscribe(has_shadow_slot_, has_shadow_cb_);
}

void frame_handler<platform::windows>::map_corner_radius(frame& f) {
    apply_corner_radius(f.corner_radius.get());
    f.corner_radius.changed.subscribe(corner_radius_slot_, corner_radius_cb_);
}

void frame_handler<platform::windows>::map_padding(frame& f) {
    apply_padding(f.padding.get());
    f.padding.changed.subscribe(padding_slot_, padding_cb_);
}

void frame_handler<platform::windows>::bind_content(frame& f, view& child) {
    f.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp

// ---------- Self-registration with the per-platform dispatch registry --

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_frame(::mpapp::view* v) {
    if (auto* fr = dynamic_cast<::mpapp::frame*>(v); fr && fr->has_handler()) {
        return fr->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_frame); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif
