// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 menu_bar handler implementation.

#include "mpapp/handlers/windows/menu_bar_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/menu_bar_item.hpp"
#include "mpapp/handlers/windows/menu_bar_item_handler.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

menu_bar_handler<platform::windows>::menu_bar_handler() {
    // mux::Controls::MenuBar requires a recent WinUI 3 — swallow the
    // hresult_class_not_registered case so the rest of the app still
    // hosts (same pattern title_bar uses).
    try {
        native_ = muxc::MenuBar{};
    } catch (...) {
        native_ = nullptr;
    }
}

menu_bar_handler<platform::windows>::~menu_bar_handler() = default;

void menu_bar_handler<platform::windows>::apply_items(const std::vector<view*>& v) {
    if (native_ == nullptr) return;
    try {
        auto items = native_.Items();
        items.Clear();
        for (view* child : v) {
            if (child == nullptr) continue;
            // The top-level entries of a WinUI MenuBar must be MenuBarItem
            // instances (not arbitrary UIElement). Resolve via direct
            // dynamic_cast to the strongly-typed child rather than the
            // ADR-0013 UIElement dispatcher.
            if (auto* mbi = dynamic_cast<menu_bar_item*>(child); mbi && mbi->has_handler()) {
                items.Append(mbi->handler().native());
            }
        }
    } catch (...) { /* ignore */ }
}

void menu_bar_handler<platform::windows>::map_items(menu_bar& b) {
    apply_items(b.items.get());
    b.items.changed.subscribe(items_slot_, items_cb_);
}

} // namespace mpapp

// --- ADR-0013 self-registration --------------------------------------------

namespace {

::winrt::Microsoft::UI::Xaml::UIElement
dispatch_menu_bar(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::menu_bar*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar_menu_bar {
    registrar_menu_bar() {
        ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_menu_bar);
    }
};

[[maybe_unused]] registrar_menu_bar _reg_mb;

} // namespace

#endif // _WIN32
