// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_menu_flyout_item handler implementation.

#include "mpapp/handlers/windows/menu_flyout_item_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

#include "mpapp/internal/basic_menu_flyout_item.hpp"

namespace mpapp::internal {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

menu_flyout_item_handler<platform::windows>::menu_flyout_item_handler() {
    native_ = muxc::MenuFlyoutItem{};
}

menu_flyout_item_handler<platform::windows>::~menu_flyout_item_handler() {
    if (native_ != nullptr && click_token_.value != 0) {
        try {
            native_.Click(click_token_);
        } catch (...) {}
        click_token_ = {};
    }
}

void menu_flyout_item_handler<platform::windows>::apply_text(const std::string& v) {
    if (native_ == nullptr) return;
    try {
        native_.Text(detail::to_hstring_utf8(v));
    } catch (...) {}
}

void menu_flyout_item_handler<platform::windows>::apply_is_enabled(bool v) {
    if (native_ == nullptr) return;
    try {
        native_.IsEnabled(v);
    } catch (...) {}
}

void menu_flyout_item_handler<platform::windows>::map_text(basic_menu_flyout_item& i) {
    owner_ = &i;
    apply_text(i.text.get());
    i.text.changed.subscribe(text_slot_, text_cb_);
    // Wire the native Click event once — the first mapper to bind
    // owns the wiring (text is always mapped first in standard order).
    if (native_ != nullptr && click_token_.value == 0) {
        basic_menu_flyout_item* target = owner_;
        click_token_ = native_.Click([target](
            ::winrt::Windows::Foundation::IInspectable const&,
            ::winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
            if (target != nullptr) {
                target->clicked.emit();
            }
        });
    }
}

void menu_flyout_item_handler<platform::windows>::map_is_enabled(basic_menu_flyout_item& i) {
    apply_is_enabled(i.is_enabled.get());
    i.is_enabled.changed.subscribe(is_enabled_slot_, is_enabled_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_menu_flyout_item(::mpapp::view* v) {
    if (auto* i = dynamic_cast<::mpapp::internal::basic_menu_flyout_item*>(v); i && i->has_handler()) {
        // MenuFlyoutItem → MenuFlyoutItemBase → Control → FrameworkElement
        // → UIElement; the implicit conversion handles the upcast.
        return i->handler().native();
    }
    return nullptr;
}

struct registrar_mfi {
    registrar_mfi() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_menu_flyout_item); }
};

[[maybe_unused]] registrar_mfi _reg;

} // namespace

#endif // _WIN32
