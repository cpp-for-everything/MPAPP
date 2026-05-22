// SPDX-License-Identifier: Apache-2.0
// WinUI 3 shape_view handler implementation.

#include "mpapp/handlers/windows/shape_view_handler.hpp"

#if defined(_WIN32)

#include <cctype>
#include <cstdint>
#include <cstdio>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

namespace mpapp {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;
namespace muxm = ::winrt::Microsoft::UI::Xaml::Media;
namespace muxs = ::winrt::Microsoft::UI::Xaml::Shapes;
namespace wui  = ::winrt::Windows::UI;

namespace {

// Parse `#RGB` / `#RRGGBB` / `#AARRGGBB`. Empty / unknown → nullopt.
struct parsed_color { std::uint8_t a, r, g, b; bool ok; };

parsed_color parse_color(const std::string& s) {
    parsed_color out{0, 0, 0, 0, false};
    if (s.empty() || s[0] != '#') return out;
    auto hex_val = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    auto byte = [&](size_t i) -> int {
        int hi = hex_val(s[i]);
        int lo = hex_val(s[i + 1]);
        if (hi < 0 || lo < 0) return -1;
        return (hi << 4) | lo;
    };
    if (s.size() == 7) {
        int r = byte(1), g = byte(3), b = byte(5);
        if (r < 0 || g < 0 || b < 0) return out;
        out = {0xFF, static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g),
               static_cast<std::uint8_t>(b), true};
    } else if (s.size() == 9) {
        int a = byte(1), r = byte(3), g = byte(5), b = byte(7);
        if (a < 0 || r < 0 || g < 0 || b < 0) return out;
        out = {static_cast<std::uint8_t>(a), static_cast<std::uint8_t>(r),
               static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b), true};
    } else if (s.size() == 4) {
        int r = hex_val(s[1]), g = hex_val(s[2]), b = hex_val(s[3]);
        if (r < 0 || g < 0 || b < 0) return out;
        out = {0xFF, static_cast<std::uint8_t>(r * 0x11),
               static_cast<std::uint8_t>(g * 0x11), static_cast<std::uint8_t>(b * 0x11), true};
    }
    return out;
}

muxm::SolidColorBrush brush_from_string(const std::string& s) {
    auto pc = parse_color(s);
    if (!pc.ok) return muxm::SolidColorBrush{};
    wui::Color c{pc.a, pc.r, pc.g, pc.b};
    return muxm::SolidColorBrush{c};
}

// Pull the first 4 floats out of an SVG-ish "M x1 y1 L x2 y2" string.
// Returns true if exactly 4 numbers were found.
bool parse_line(const std::string& data, double& x1, double& y1, double& x2, double& y2) {
    double v[4] = {0, 0, 0, 0};
    int n = 0;
    const char* p = data.c_str();
    while (*p && n < 4) {
        while (*p && !(std::isdigit(static_cast<unsigned char>(*p)) || *p == '-' || *p == '+' || *p == '.')) ++p;
        if (!*p) break;
        char* end = nullptr;
        v[n++] = std::strtod(p, &end);
        if (end == p) break;
        p = end;
    }
    if (n != 4) return false;
    x1 = v[0]; y1 = v[1]; x2 = v[2]; y2 = v[3];
    return true;
}

} // namespace

shape_view_handler<platform::windows>::shape_view_handler() {
    native_ = muxc::Border{};
    rebuild_shape(shape_kind::rectangle);
}

shape_view_handler<platform::windows>::~shape_view_handler() = default;

void shape_view_handler<platform::windows>::rebuild_shape(shape_kind k) {
    if (native_ == nullptr) return;
    muxs::Shape new_shape{nullptr};
    switch (k) {
        case shape_kind::ellipse: new_shape = muxs::Ellipse{};   break;
        case shape_kind::line:    new_shape = muxs::Line{};      break;
        case shape_kind::rectangle:
        case shape_kind::polygon: // v1: render as bounding rect
        case shape_kind::path:    // v1: render as bounding rect
        default:                  new_shape = muxs::Rectangle{}; break;
    }
    shape_ = new_shape;
    if (auto fe = shape_.try_as<mux::FrameworkElement>()) {
        fe.HorizontalAlignment(mux::HorizontalAlignment::Stretch);
        fe.VerticalAlignment(mux::VerticalAlignment::Stretch);
    }
    native_.Child(shape_);
    apply_paint();
    if (bound_ != nullptr) apply_data(bound_->data.get());
}

void shape_view_handler<platform::windows>::apply_paint() {
    if (shape_ == nullptr || bound_ == nullptr) return;
    shape_.Fill(brush_from_string(bound_->fill.get()));
    shape_.Stroke(brush_from_string(bound_->stroke.get()));
    shape_.StrokeThickness(bound_->stroke_thickness.get());
}

void shape_view_handler<platform::windows>::apply_data(const std::string& v) {
    if (shape_ == nullptr || bound_ == nullptr) return;
    if (auto line = shape_.try_as<muxs::Line>()) {
        double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
        if (parse_line(v, x1, y1, x2, y2)) {
            line.X1(x1); line.Y1(y1); line.X2(x2); line.Y2(y2);
        }
    }
    // Rectangle / Ellipse / Path / Polygon ignore `data` in v1.
}

void shape_view_handler<platform::windows>::opacity_cb_t::operator()(double v) const {
    if (self->native_ != nullptr) self->native_.Opacity(v);
}

void shape_view_handler<platform::windows>::map_kind(shape_view& s) {
    bound_ = &s;
    rebuild_shape(s.kind.get());
    s.kind.changed.subscribe(kind_slot_, kind_cb_);
}
void shape_view_handler<platform::windows>::map_data(shape_view& s) {
    apply_data(s.data.get());
    s.data.changed.subscribe(data_slot_, data_cb_);
}
void shape_view_handler<platform::windows>::map_fill(shape_view& s) {
    apply_paint();
    s.fill.changed.subscribe(fill_slot_, fill_cb_);
}
void shape_view_handler<platform::windows>::map_stroke(shape_view& s) {
    apply_paint();
    s.stroke.changed.subscribe(stroke_slot_, stroke_cb_);
}
void shape_view_handler<platform::windows>::map_stroke_thickness(shape_view& s) {
    apply_paint();
    s.stroke_thickness.changed.subscribe(stroke_thick_slot_, stroke_thick_cb_);
}
void shape_view_handler<platform::windows>::map_opacity(shape_view& s) {
    if (native_ != nullptr) native_.Opacity(s.opacity.get());
    s.opacity.changed.subscribe(opacity_slot_, opacity_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_shape_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::shape_view*>(v); w && w->has_sv_handler()) {
        return w->sv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_shape_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
