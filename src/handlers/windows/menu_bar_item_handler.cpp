// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_menu_bar_item handler implementation.

#include "mpapp/handlers/windows/menu_bar_item_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "winrt_strings.hpp"

namespace mpapp::internal {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

menu_bar_item_handler<platform::windows>::menu_bar_item_handler() {
    try {
        native_ = muxc::MenuBarItem{};
    } catch (...) {
        native_ = nullptr;
    }
}

menu_bar_item_handler<platform::windows>::~menu_bar_item_handler() = default;

void menu_bar_item_handler<platform::windows>::apply_title(const std::string& v) {
    if (native_ == nullptr) return;
    try {
        native_.Title(detail::to_hstring_utf8(v));
    } catch (...) { /* ignore */ }
}

void menu_bar_item_handler<platform::windows>::apply_items(const std::vector<view*>& v) {
    if (native_ == nullptr) return;
    try {
        auto items = native_.Items();
        items.Clear();
        // The Items collection of a WinUI MenuBarItem expects
        // MenuFlyoutItemBase descendants (MenuFlyoutItem,
        // MenuFlyoutSeparator, MenuFlyoutSubItem). The M-04c basic_menu_flyout
        // family will populate these via the ADR-0013 registry; for the
        // M-04b baseline we accept anything that resolves to a
        // `MenuFlyoutItemBase` via the dispatch registry and silently
        // drop the rest. The registry stores everything as `UIElement`,
        // so we runtime-cast back to the expected base.
        for (view* child : v) {
            if (child == nullptr) continue;
            auto el = ::mpapp::detail::windows_dispatch::dispatch(child);
            if (el == nullptr) continue;
            if (auto base = el.try_as<muxc::MenuFlyoutItemBase>()) {
                items.Append(base);
            }
        }
    } catch (...) { /* ignore */ }
}

void menu_bar_item_handler<platform::windows>::map_title(basic_menu_bar_item& m) {
    apply_title(m.title.get());
    m.title.changed.subscribe(title_slot_, title_cb_);
}

void menu_bar_item_handler<platform::windows>::map_items(basic_menu_bar_item& m) {
    apply_items(m.items.get());
    m.items.changed.subscribe(items_slot_, items_cb_);
}

} // namespace mpapp::internal
// --- ADR-0013 self-registration --------------------------------------------

namespace {

::winrt::Microsoft::UI::Xaml::UIElement
dispatch_menu_bar_item(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_menu_bar_item*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar_menu_bar_item {
    registrar_menu_bar_item() {
        ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_menu_bar_item);
    }
};

[[maybe_unused]] registrar_menu_bar_item _reg_mbi;

} // namespace

#endif // _WIN32
