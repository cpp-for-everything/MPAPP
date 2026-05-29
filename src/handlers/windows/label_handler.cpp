// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0003 — WinUI 3 basic_button spike.
//
// Implementation of `label_handler<platform::windows>` against C++/WinRT
// WinUI 3 `TextBlock`.

#include "mpapp/handlers/windows/label_handler.hpp"

#if defined(_WIN32)

#include <cstdint>

#include <winrt/base.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Text.h>
#include <winrt/Windows.UI.h>

#include "winrt_strings.hpp"
#include "mpapp/handlers/windows/gesture_attach.hpp"

namespace mpapp::internal {

void label_handler<platform::windows>::apply_text(std::string_view text) {
    if (native_ != nullptr) {
        native_.Text(detail::to_hstring_utf8(text));
    }
}

void label_handler<platform::windows>::apply_font_size(double v) {
    if (native_ != nullptr && v > 0.0) {
        native_.FontSize(v);
    }
}

void label_handler<platform::windows>::apply_font_bold(bool v) {
    if (native_ != nullptr) {
        native_.FontWeight(v ? winrt::Microsoft::UI::Text::FontWeights::Bold()
                             : winrt::Microsoft::UI::Text::FontWeights::Normal());
    }
}

void label_handler<platform::windows>::apply_font_family(std::string_view v) {
    if (native_ != nullptr && !v.empty()) {
        native_.FontFamily(winrt::Microsoft::UI::Xaml::Media::FontFamily{
            detail::to_hstring_utf8(v)});
    }
}

label_handler<platform::windows>::label_handler() {
    native_ = winrt::Microsoft::UI::Xaml::Controls::TextBlock{};
}

label_handler<platform::windows>::~label_handler() = default;

void label_handler<platform::windows>::map_text(basic_label& l) {
    apply_text(l.text.get());
    l.text.changed.subscribe(text_slot_, text_callback_);
}

void label_handler<platform::windows>::map_font_size(basic_label& l) {
    apply_font_size(l.font_size.get());
    l.font_size.changed.subscribe(fsize_slot_, fsize_callback_);
}

void label_handler<platform::windows>::map_font_bold(basic_label& l) {
    apply_font_bold(l.font_bold.get());
    l.font_bold.changed.subscribe(fbold_slot_, fbold_callback_);
}

void label_handler<platform::windows>::apply_text_color(const color& c) {
    if (native_ == nullptr || c.a <= 0.0) return;
    auto to8 = [](double v) -> uint8_t {
        if (v < 0.0) v = 0.0;
        if (v > 1.0) v = 1.0;
        return static_cast<uint8_t>(v * 255.0 + 0.5);
    };
    winrt::Windows::UI::Color col{};
    col.A = to8(c.a);
    col.R = to8(c.r);
    col.G = to8(c.g);
    col.B = to8(c.b);
    native_.Foreground(winrt::Microsoft::UI::Xaml::Media::SolidColorBrush{col});
}

void label_handler<platform::windows>::map_font_family(basic_label& l) {
    apply_font_family(l.font_family.get());
    l.font_family.changed.subscribe(ffamily_slot_, ffamily_callback_);
}

void label_handler<platform::windows>::map_text_color(basic_label& l) {
    apply_text_color(l.text_color.get());
    l.text_color.changed.subscribe(tcolor_slot_, tcolor_callback_);
}

void label_handler<platform::windows>::map_gestures(basic_label& x) {
    windows_gestures::attach(native_, x);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/internal/basic_label.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_label(::mpapp::view* v) {
    if (auto* l = dynamic_cast<::mpapp::internal::basic_label*>(v); l && l->has_handler()) {
        return l->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_label); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
