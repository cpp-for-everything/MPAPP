// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_flyout_view handler implementation.

#include "mpapp/handlers/windows/flyout_view_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

namespace mpapp::internal {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

flyout_view_handler<platform::windows>::flyout_view_handler() {
    native_ = muxc::NavigationView{};
    // Hide the built-in chrome (back/forward, settings, search) — the
    // M-04b spike just exposes the two-pane layout, not the
    // NavigationView's full app-basic_shell affordances.
    native_.IsBackButtonVisible(muxc::NavigationViewBackButtonVisible::Collapsed);
    native_.IsSettingsVisible(false);
    native_.PaneDisplayMode(muxc::NavigationViewPaneDisplayMode::Auto);
    // is_presented starts closed (matches the Observable default).
    native_.IsPaneOpen(false);
}

flyout_view_handler<platform::windows>::~flyout_view_handler() = default;

void flyout_view_handler<platform::windows>::apply_flyout(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    view* raw = v.get();
    if (raw != nullptr) {
        if (auto el = detail::windows_dispatch::dispatch(raw); el != nullptr) {
            native_.PaneCustomContent(el);
            return;
        }
    }
    native_.PaneCustomContent(nullptr);
}

void flyout_view_handler<platform::windows>::apply_detail(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    view* raw = v.get();
    if (raw != nullptr) {
        if (auto el = detail::windows_dispatch::dispatch(raw); el != nullptr) {
            native_.Content(el);
            return;
        }
    }
    native_.Content(nullptr);
}

void flyout_view_handler<platform::windows>::apply_is_presented(bool v) {
    if (native_ == nullptr) return;
    native_.IsPaneOpen(v);
}

void flyout_view_handler<platform::windows>::map_flyout(basic_flyout_view& f) {
    apply_flyout(f.flyout.get());
    f.flyout.changed.subscribe(flyout_slot_, flyout_cb_);
}

void flyout_view_handler<platform::windows>::map_detail(basic_flyout_view& f) {
    apply_detail(f.detail.get());
    f.detail.changed.subscribe(detail_slot_, detail_cb_);
}

void flyout_view_handler<platform::windows>::map_is_presented(basic_flyout_view& f) {
    apply_is_presented(f.is_presented.get());
    f.is_presented.changed.subscribe(is_presented_slot_, is_presented_cb_);
}

void flyout_view_handler<platform::windows>::bind_flyout(basic_flyout_view& f, view& child) {
    f.flyout.set(std::shared_ptr<view>(&child, [](view*){}));
}

void flyout_view_handler<platform::windows>::bind_detail(basic_flyout_view& f, view& child) {
    f.detail.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp::internal
// ----- ADR-0013 self-registration --------------------------------------

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_flyout_view(::mpapp::view* v) {
    if (auto* f = dynamic_cast<::mpapp::internal::basic_flyout_view*>(v); f && f->has_handler()) {
        return f->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_flyout_view);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
