// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 border handler implementation.

#include "mpapp/handlers/windows/border_handler.hpp"

#if defined(_WIN32)

#include <cstdint>
#include <cstdlib>
#include <string>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.h>
#include <winrt/Microsoft.UI.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "mpapp/box_view.hpp"
#include "mpapp/button.hpp"
#include "mpapp/check_box.hpp"
#include "mpapp/editor.hpp"
#include "mpapp/entry.hpp"
#include "mpapp/handlers/windows/box_view_handler.hpp"
#include "mpapp/handlers/windows/button_handler.hpp"
#include "mpapp/handlers/windows/check_box_handler.hpp"
#include "mpapp/handlers/windows/editor_handler.hpp"
#include "mpapp/handlers/windows/entry_handler.hpp"
#include "mpapp/handlers/windows/label_handler.hpp"
#include "mpapp/handlers/windows/radio_button_handler.hpp"
#include "mpapp/handlers/windows/slider_handler.hpp"
#include "mpapp/handlers/windows/stack_layout_handler.hpp"
#include "mpapp/handlers/windows/stepper_handler.hpp"
#include "mpapp/handlers/windows/switch_handler.hpp"
#include "mpapp/label.hpp"
#include "mpapp/radio_button.hpp"
#include "mpapp/slider.hpp"
#include "mpapp/stack_layout.hpp"
#include "mpapp/stepper.hpp"
#include "mpapp/switch_.hpp"

namespace mpapp {

namespace muxc  = ::winrt::Microsoft::UI::Xaml::Controls;
namespace muxm  = ::winrt::Microsoft::UI::Xaml::Media;
namespace muxui = ::winrt::Windows::UI;

namespace {

// Tiny parser: turn a brush_ref name into a Color. Accepts #RRGGBB,
// #AARRGGBB, or common CSS-ish color words. Defaults to black on miss.
muxui::Color parse_color(const std::string& name) {
    muxui::Color out{255, 0, 0, 0};
    if (name.empty()) return out;
    if (name[0] == '#') {
        // #RRGGBB or #AARRGGBB
        uint32_t v = static_cast<uint32_t>(std::strtoul(name.c_str() + 1, nullptr, 16));
        if (name.size() == 7) { // RRGGBB
            out.A = 255;
            out.R = static_cast<uint8_t>((v >> 16) & 0xFF);
            out.G = static_cast<uint8_t>((v >>  8) & 0xFF);
            out.B = static_cast<uint8_t>( v        & 0xFF);
        } else if (name.size() == 9) { // AARRGGBB
            out.A = static_cast<uint8_t>((v >> 24) & 0xFF);
            out.R = static_cast<uint8_t>((v >> 16) & 0xFF);
            out.G = static_cast<uint8_t>((v >>  8) & 0xFF);
            out.B = static_cast<uint8_t>( v        & 0xFF);
        }
        return out;
    }
    // Named colors (small set; expand as the design language solidifies).
    if (name == "Red")    return {255, 220,  50,  50};
    if (name == "Green")  return {255,  80, 180,  80};
    if (name == "Blue")   return {255,  60, 120, 220};
    if (name == "Black")  return {255,   0,   0,   0};
    if (name == "White")  return {255, 255, 255, 255};
    if (name == "Gray")   return {255, 128, 128, 128};
    if (name == "Teal")   return {255,   0, 150, 165};
    return out;
}

// Parse "Rectangle", "RoundRectangle(12)", or "RoundRectangle(12,4,8,4)"
// into a uniform-or-per-corner CornerRadius. Unknown shapes → 0.
::winrt::Microsoft::UI::Xaml::CornerRadius parse_corners(const std::string& desc) {
    ::winrt::Microsoft::UI::Xaml::CornerRadius out{};
    auto paren = desc.find('(');
    if (paren == std::string::npos) return out;
    auto close = desc.find(')', paren + 1);
    if (close == std::string::npos) return out;
    std::string args = desc.substr(paren + 1, close - paren - 1);
    double values[4]{};
    int n = 0;
    std::string cur;
    for (char c : args) {
        if (c == ',') {
            if (n < 4) values[n++] = std::atof(cur.c_str());
            cur.clear();
        } else if (c != ' ') {
            cur.push_back(c);
        }
    }
    if (!cur.empty() && n < 4) values[n++] = std::atof(cur.c_str());
    if (n == 1) {
        out.TopLeft = out.TopRight = out.BottomRight = out.BottomLeft = values[0];
    } else if (n == 4) {
        out.TopLeft = values[0];
        out.TopRight = values[1];
        out.BottomRight = values[2];
        out.BottomLeft = values[3];
    }
    return out;
}

} // namespace

border_handler<platform::windows>::border_handler() {
    native_ = muxc::Border{};
    // Default border (1 px) so it's visible even before stroke is set.
    native_.BorderThickness({1.0, 1.0, 1.0, 1.0});
}

border_handler<platform::windows>::~border_handler() = default;

void border_handler<platform::windows>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    view* raw = v.get();
    if (raw == nullptr) { native_.Child(nullptr); return; }
    if (auto el = detail::windows_dispatch::dispatch(raw); el != nullptr) { native_.Child(el); return; }
    if (auto* sl = dynamic_cast<stack_layout*>(raw); sl && sl->has_handler()) { native_.Child(sl->handler().native()); return; }
    if (auto* bn = dynamic_cast<button*>(raw);       bn && bn->has_handler()) { native_.Child(bn->handler().native()); return; }
    if (auto* l  = dynamic_cast<label*>(raw);        l  && l->has_handler())  { native_.Child(l->handler().native());  return; }
    if (auto* e  = dynamic_cast<entry*>(raw);        e  && e->has_handler())  { native_.Child(e->handler().native());  return; }
    if (auto* sw = dynamic_cast<switch_*>(raw);      sw && sw->has_handler()) { native_.Child(sw->handler().native()); return; }
    if (auto* cb = dynamic_cast<check_box*>(raw);    cb && cb->has_handler()) { native_.Child(cb->handler().native()); return; }
    if (auto* rb = dynamic_cast<radio_button*>(raw); rb && rb->has_handler()) { native_.Child(rb->handler().native()); return; }
    if (auto* sl2= dynamic_cast<slider*>(raw);       sl2&& sl2->has_handler()){ native_.Child(sl2->handler().native()); return; }
    if (auto* st = dynamic_cast<stepper*>(raw);      st && st->has_handler()) { native_.Child(st->handler().native()); return; }
    if (auto* ed = dynamic_cast<editor*>(raw);       ed && ed->has_handler()) { native_.Child(ed->handler().native()); return; }
    if (auto* bx = dynamic_cast<box_view*>(raw);     bx && bx->has_handler()) { native_.Child(bx->handler().native()); return; }
}

void border_handler<platform::windows>::apply_padding(const thickness& t) {
    if (native_ == nullptr) return;
    ::winrt::Microsoft::UI::Xaml::Thickness wt{t.left, t.top, t.right, t.bottom};
    native_.Padding(wt);
}

void border_handler<platform::windows>::apply_stroke(const brush_ref& b) {
    if (native_ == nullptr) return;
    muxm::SolidColorBrush brush{parse_color(b.name)};
    native_.BorderBrush(brush);
}

void border_handler<platform::windows>::apply_stroke_thickness(double t) {
    if (native_ == nullptr) return;
    native_.BorderThickness({t, t, t, t});
}

void border_handler<platform::windows>::apply_stroke_shape(const stroke_shape_desc& s) {
    if (native_ == nullptr) return;
    native_.CornerRadius(parse_corners(s.descriptor));
}

void border_handler<platform::windows>::map_content(border& b)          { apply_content(b.content.get()); b.content.changed.subscribe(content_slot_, content_cb_); }
void border_handler<platform::windows>::map_padding(border& b)          { apply_padding(b.padding.get()); b.padding.changed.subscribe(padding_slot_, padding_cb_); }
void border_handler<platform::windows>::map_stroke(border& b)           { apply_stroke(b.stroke.get()); b.stroke.changed.subscribe(stroke_slot_, stroke_cb_); }
void border_handler<platform::windows>::map_stroke_thickness(border& b) { apply_stroke_thickness(b.stroke_thickness.get()); b.stroke_thickness.changed.subscribe(stroke_thick_slot_, stroke_thick_cb_); }
void border_handler<platform::windows>::map_stroke_shape(border& b)     { apply_stroke_shape(b.stroke_shape.get()); b.stroke_shape.changed.subscribe(stroke_shape_slot_, stroke_shape_cb_); }

void border_handler<platform::windows>::bind_content(border& b, view& child) {
    b.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp

#endif // _WIN32
