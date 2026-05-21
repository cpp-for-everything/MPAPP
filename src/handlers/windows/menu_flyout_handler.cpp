// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 menu_flyout handler implementation.

#include "mpapp/handlers/windows/menu_flyout_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "mpapp/menu_flyout.hpp"
#include "mpapp/view.hpp"

namespace mpapp {

namespace muxc  = ::winrt::Microsoft::UI::Xaml::Controls;
namespace muxcp = ::winrt::Microsoft::UI::Xaml::Controls::Primitives;

menu_flyout_handler<platform::windows>::menu_flyout_handler() {
    native_ = muxc::MenuFlyout{};
}

menu_flyout_handler<platform::windows>::~menu_flyout_handler() = default;

void menu_flyout_handler<platform::windows>::apply_items(const std::vector<view*>& v) {
    if (native_ == nullptr) return;
    try {
        auto items = native_.Items();
        items.Clear();
        for (view* child : v) {
            if (child == nullptr) continue;
            // ADR-0013 — resolve through the registry. The
            // menu_flyout_item / _separator / _sub_item handlers each
            // expose their MenuFlyoutItemBase derivative as their
            // UIElement-castable native() return value.
            auto el = detail::windows_dispatch::dispatch(child);
            if (el == nullptr) continue;
            // MenuFlyout::Items() takes MenuFlyoutItemBase; UIElement
            // does not implicitly convert. try_as<> handles the
            // downcast — every child registered through the
            // menu_flyout_* handlers returns a MenuFlyoutItemBase.
            auto base = el.try_as<muxc::MenuFlyoutItemBase>();
            if (base != nullptr) {
                items.Append(base);
            }
        }
    } catch (...) {}
}

void menu_flyout_handler<platform::windows>::apply_is_open(bool v) {
    if (native_ == nullptr) return;
    // M-04b: track the state without anchoring to a real target —
    // ShowAt requires a FrameworkElement anchor which is a host
    // concern, lands with the M-05 ContextFlyout wiring. Until then
    // the Observable carries the intent and the unit tests verify
    // the mapper plumbing; nothing visible yet.
    (void)v;
}

void menu_flyout_handler<platform::windows>::map_items(menu_flyout& f) {
    apply_items(f.items.get());
    f.items.changed.subscribe(items_slot_, items_cb_);
}

void menu_flyout_handler<platform::windows>::map_is_open(menu_flyout& f) {
    apply_is_open(f.is_open.get());
    f.is_open.changed.subscribe(is_open_slot_, is_open_cb_);
}

} // namespace mpapp

// ---------- Self-registration with the per-platform dispatch registry --

namespace {

// menu_flyout itself is not a UIElement — `MenuFlyout` derives from
// `FlyoutBase` which is a `DependencyObject`, not a `UIElement`. It
// cannot be placed in a container's child list, so the dispatcher
// returns nullptr. The registrar is still installed for ADR-0013
// uniformity and so the same shape applies if menu_flyout later
// grows a host-element wrapper.
::winrt::Microsoft::UI::Xaml::UIElement dispatch_menu_flyout(::mpapp::view* /*v*/) {
    return nullptr;
}

struct registrar_mf {
    registrar_mf() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_menu_flyout); }
};

[[maybe_unused]] registrar_mf _reg;

} // namespace

#endif // _WIN32
