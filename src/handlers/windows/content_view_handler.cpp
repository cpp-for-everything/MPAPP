// SPDX-License-Identifier: Apache-2.0
// WinUI 3 content_view handler implementation.

#include "mpapp/handlers/windows/content_view_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

content_view_handler<platform::windows>::content_view_handler() {
    native_ = muxc::ContentControl{};
}

content_view_handler<platform::windows>::~content_view_handler() = default;

void content_view_handler<platform::windows>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    view* raw = v.get();
    if (raw == nullptr) { native_.Content(nullptr); return; }
    // ADR-0013: ask the per-platform dispatch registry. Each widget's .cpp
    // self-registers a dispatcher that returns its native UIElement; the
    // registry tries each registered dispatcher in order and returns the
    // first non-null. This replaces the legacy dynamic_cast chain that
    // had to be edited every time a new widget landed.
    if (auto el = detail::windows_dispatch::dispatch(raw); el != nullptr) {
        native_.Content(el);
        return;
    }
}

void content_view_handler<platform::windows>::map_content(content_view& c) {
    apply_content(c.content.get());
    c.content.changed.subscribe(content_slot_, content_cb_);
}

void content_view_handler<platform::windows>::bind_content(content_view& c, view& child) {
    c.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp

// ---------- Self-registration with the per-platform dispatch registry --
// ContentView can host any widget AND can be hosted as a widget. The
// self-registrar lets parent containers (stack_layout, scroll_view, etc.)
// place a content_view as their child via the same ADR-0013 path.
#include "mpapp/content_view.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_content_view(::mpapp::view* v) {
    if (auto* cv = dynamic_cast<::mpapp::content_view*>(v); cv && cv->has_handler()) {
        return cv->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_content_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
