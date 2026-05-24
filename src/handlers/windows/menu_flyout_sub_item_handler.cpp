// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_menu_flyout_sub_item handler implementation.

#include "mpapp/handlers/windows/menu_flyout_sub_item_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

#include "mpapp/internal/basic_menu_flyout_sub_item.hpp"
#include "mpapp/view.hpp"

namespace mpapp::internal {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

menu_flyout_sub_item_handler<platform::windows>::menu_flyout_sub_item_handler() {
    native_ = muxc::MenuFlyoutSubItem{};
}

menu_flyout_sub_item_handler<platform::windows>::~menu_flyout_sub_item_handler() = default;

void menu_flyout_sub_item_handler<platform::windows>::apply_text(const std::string& v) {
    if (native_ == nullptr) return;
    try {
        native_.Text(detail::to_hstring_utf8(v));
    } catch (...) {}
}

void menu_flyout_sub_item_handler<platform::windows>::apply_items(const std::vector<view*>& v) {
    if (native_ == nullptr) return;
    try {
        auto items = native_.Items();
        items.Clear();
        for (view* child : v) {
            if (child == nullptr) continue;
            auto el = detail::windows_dispatch::dispatch(child);
            if (el == nullptr) continue;
            auto base = el.try_as<muxc::MenuFlyoutItemBase>();
            if (base != nullptr) {
                items.Append(base);
            }
        }
    } catch (...) {}
}

void menu_flyout_sub_item_handler<platform::windows>::map_text(basic_menu_flyout_sub_item& s) {
    apply_text(s.text.get());
    s.text.changed.subscribe(text_slot_, text_cb_);
}

void menu_flyout_sub_item_handler<platform::windows>::map_items(basic_menu_flyout_sub_item& s) {
    apply_items(s.items.get());
    s.items.changed.subscribe(items_slot_, items_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_menu_flyout_sub_item(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::internal::basic_menu_flyout_sub_item*>(v); s && s->has_handler()) {
        return s->handler().native();
    }
    return nullptr;
}

struct registrar_mfsi {
    registrar_mfsi() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_menu_flyout_sub_item); }
};

[[maybe_unused]] registrar_mfsi _reg;

} // namespace

#endif // _WIN32
