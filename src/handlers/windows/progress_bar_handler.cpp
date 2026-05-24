// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_progress_bar handler implementation.

#include "mpapp/handlers/windows/progress_bar_handler.hpp"

#if defined(_WIN32)

#include <cstdint>
#include <cstdlib>
#include <string>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>

namespace mpapp::internal {

namespace muxc  = ::winrt::Microsoft::UI::Xaml::Controls;
namespace muxm  = ::winrt::Microsoft::UI::Xaml::Media;
namespace muxui = ::winrt::Windows::UI;

namespace {

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
        } else { return fallback; }
        return out;
    }
    if (name == "Red")   return {255, 220,  50,  50};
    if (name == "Green") return {255,  80, 180,  80};
    if (name == "Blue")  return {255,  60, 120, 220};
    if (name == "Black") return {255,   0,   0,   0};
    if (name == "White") return {255, 255, 255, 255};
    if (name == "Gray")  return {255, 128, 128, 128};
    if (name == "Teal")  return {255,   0, 150, 165};
    return fallback;
}

} // namespace

progress_bar_handler<platform::windows>::progress_bar_handler() {
    native_ = muxc::ProgressBar{};
    native_.Minimum(0.0);
    native_.Maximum(1.0);
}

progress_bar_handler<platform::windows>::~progress_bar_handler() = default;

void progress_bar_handler<platform::windows>::apply_progress(double v) {
    if (native_ == nullptr) return;
    if (v < 0.0) v = 0.0;
    if (v > 1.0) v = 1.0;
    native_.Value(v);
}

void progress_bar_handler<platform::windows>::apply_color(const brush_ref& b) {
    if (native_ == nullptr) return;
    muxm::SolidColorBrush brush{parse_color(b.name, {255, 0, 120, 215})};
    native_.Foreground(brush);
}

void progress_bar_handler<platform::windows>::apply_background_color(const brush_ref& b) {
    if (native_ == nullptr) return;
    muxm::SolidColorBrush brush{parse_color(b.name, {255, 240, 240, 240})};
    native_.Background(brush);
}

void progress_bar_handler<platform::windows>::map_progress(basic_progress_bar& p) {
    apply_progress(p.progress.get());
    p.progress.changed.subscribe(progress_slot_, progress_cb_);
}
void progress_bar_handler<platform::windows>::map_color(basic_progress_bar& p) {
    apply_color(p.color.get());
    p.color.changed.subscribe(color_slot_, color_cb_);
}
void progress_bar_handler<platform::windows>::map_background_color(basic_progress_bar& p) {
    apply_background_color(p.background_color.get());
    p.background_color.changed.subscribe(bg_slot_, bg_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_progress_bar so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/internal/basic_progress_bar.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_progress_bar(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_progress_bar*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_progress_bar); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
