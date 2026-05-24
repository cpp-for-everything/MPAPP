// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_refresh_view handler implementation.

#include "mpapp/handlers/windows/refresh_view_handler.hpp"

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

#include "mpapp/handlers/windows/widget_dispatch.hpp"

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
    if (name == "Red")        return {255, 220,  50,  50};
    if (name == "Green")      return {255,  80, 180,  80};
    if (name == "Blue")       return {255,  60, 120, 220};
    if (name == "Black")      return {255,   0,   0,   0};
    if (name == "White")      return {255, 255, 255, 255};
    if (name == "Gray")       return {255, 128, 128, 128};
    if (name == "Teal")       return {255,   0, 150, 165};
    if (name == "DodgerBlue") return {255,  30, 144, 255};
    return out;
}

} // namespace

refresh_view_handler<platform::windows>::refresh_view_handler() {
    native_  = muxc::Grid{};
    spinner_ = muxc::ProgressRing{};
    spinner_.HorizontalAlignment(::winrt::Microsoft::UI::Xaml::HorizontalAlignment::Center);
    spinner_.VerticalAlignment(::winrt::Microsoft::UI::Xaml::VerticalAlignment::Center);
    spinner_.IsActive(false);
    spinner_.Visibility(::winrt::Microsoft::UI::Xaml::Visibility::Collapsed);
    // Spinner sits on top; the content (if any) is inserted at index 0
    // beneath it. This way clearing+rebuilding the children is trivial:
    // Clear() drops everything, then we Append the new content first
    // (index 0) and the spinner second (index 1).
}

refresh_view_handler<platform::windows>::~refresh_view_handler() = default;

void refresh_view_handler<platform::windows>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    auto children = native_.Children();
    children.Clear();
    view* raw = v.get();
    if (raw != nullptr) {
        if (auto el = detail::windows_dispatch::dispatch(raw); el != nullptr) {
            children.Append(el);
        }
    }
    // Re-attach the spinner overlay last so it stacks over the content.
    children.Append(spinner_);
}

void refresh_view_handler<platform::windows>::apply_is_refreshing(bool v) {
    if (spinner_ == nullptr) return;
    spinner_.IsActive(v);
    spinner_.Visibility(v ? ::winrt::Microsoft::UI::Xaml::Visibility::Visible
                          : ::winrt::Microsoft::UI::Xaml::Visibility::Collapsed);
}

void refresh_view_handler<platform::windows>::apply_refresh_color(const brush_ref& b) {
    if (spinner_ == nullptr) return;
    muxm::SolidColorBrush brush{parse_color(b.name)};
    spinner_.Foreground(brush);
}

void refresh_view_handler<platform::windows>::map_content(basic_refresh_view& r) {
    apply_content(r.content.get());
    r.content.changed.subscribe(content_slot_, content_cb_);
}

void refresh_view_handler<platform::windows>::map_is_refreshing(basic_refresh_view& r) {
    apply_is_refreshing(r.is_refreshing.get());
    r.is_refreshing.changed.subscribe(is_refreshing_slot_, is_refreshing_cb_);
}

void refresh_view_handler<platform::windows>::map_refresh_color(basic_refresh_view& r) {
    apply_refresh_color(r.refresh_color.get());
    r.refresh_color.changed.subscribe(refresh_color_slot_, refresh_color_cb_);
}

void refresh_view_handler<platform::windows>::bind_content(basic_refresh_view& r, view& child) {
    r.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

// ----- ADR-0013 self-registration --------------------------------------

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_refresh_view(::mpapp::view* v) {
    if (auto* r = dynamic_cast<::mpapp::internal::basic_refresh_view*>(v); r && r->has_handler()) {
        return r->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_refresh_view);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

} // namespace mpapp::internal
#endif // _WIN32
