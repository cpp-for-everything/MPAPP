// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_activity_indicator handler implementation.

#include "mpapp/handlers/windows/activity_indicator_handler.hpp"

#if defined(_WIN32)

#include <cstdint>
#include <cstdlib>
#include <string>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>

namespace mpapp::internal {

namespace muxc  = ::winrt::Microsoft::UI::Xaml::Controls;
namespace muxm  = ::winrt::Microsoft::UI::Xaml::Media;
namespace muxui = ::winrt::Windows::UI;

namespace {

muxui::Color parse_color(const std::string& name) {
    muxui::Color out{255, 0, 120, 215};  // Windows accent default
    if (name.empty()) return out;
    if (name[0] == '#') {
        uint32_t v = static_cast<uint32_t>(std::strtoul(name.c_str() + 1, nullptr, 16));
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
        }
        return out;
    }
    if (name == "Red")   return {255, 220,  50,  50};
    if (name == "Green") return {255,  80, 180,  80};
    if (name == "Blue")  return {255,  60, 120, 220};
    if (name == "Black") return {255,   0,   0,   0};
    if (name == "White") return {255, 255, 255, 255};
    if (name == "Teal")  return {255,   0, 150, 165};
    return out;
}

} // namespace

activity_indicator_handler<platform::windows>::activity_indicator_handler() {
    native_ = muxc::ProgressRing{};
}

activity_indicator_handler<platform::windows>::~activity_indicator_handler() = default;

void activity_indicator_handler<platform::windows>::apply_is_running(bool v) {
    if (native_ == nullptr) return;
    native_.IsActive(v);
    native_.Visibility(v ? ::winrt::Microsoft::UI::Xaml::Visibility::Visible
                         : ::winrt::Microsoft::UI::Xaml::Visibility::Collapsed);
}

void activity_indicator_handler<platform::windows>::apply_color(const brush_ref& b) {
    if (native_ == nullptr) return;
    muxm::SolidColorBrush brush{parse_color(b.name)};
    native_.Foreground(brush);
}

void activity_indicator_handler<platform::windows>::map_is_running(basic_activity_indicator& a) {
    apply_is_running(a.is_running.get());
    a.is_running.changed.subscribe(is_running_slot_, is_running_cb_);
}

void activity_indicator_handler<platform::windows>::map_color(basic_activity_indicator& a) {
    apply_color(a.color.get());
    a.color.changed.subscribe(color_slot_, color_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_activity_indicator so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/internal/basic_activity_indicator.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_activity_indicator(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_activity_indicator*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_activity_indicator); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
