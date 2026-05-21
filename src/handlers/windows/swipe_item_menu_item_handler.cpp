// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 swipe_item_menu_item handler implementation.

#include "mpapp/handlers/windows/swipe_item_menu_item_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

swipe_item_menu_item_handler<platform::windows>::swipe_item_menu_item_handler() {
    try {
        native_ = muxc::Button{};
    } catch (...) {
        native_ = nullptr;
    }
}

swipe_item_menu_item_handler<platform::windows>::~swipe_item_menu_item_handler() {
    if (native_ != nullptr && click_token_.value != 0) {
        try { native_.Click(click_token_); } catch (...) {}
        click_token_ = {};
    }
}

void swipe_item_menu_item_handler<platform::windows>::apply_text(const std::string& v) {
    if (native_ == nullptr) return;
    try {
        native_.Content(::winrt::box_value(detail::to_hstring_utf8(v)));
    } catch (...) {}
}

void swipe_item_menu_item_handler<platform::windows>::apply_icon_uri(const std::string& v) {
    // Icon plumbing is symbolic — the real `image_source` variant lands
    // alongside the broader image-source resolver. For now, just touching
    // the icon URI is a no-op on the WinUI side; the value is still
    // captured + observable for tests / future consumers.
    (void)v;
}

void swipe_item_menu_item_handler<platform::windows>::map_text(swipe_item_menu_item& m) {
    apply_text(m.text.get());
    m.text.changed.subscribe(text_slot_, text_cb_);
}

void swipe_item_menu_item_handler<platform::windows>::map_icon_uri(swipe_item_menu_item& m) {
    apply_icon_uri(m.icon_uri.get());
    m.icon_uri.changed.subscribe(icon_slot_, icon_cb_);
}

void swipe_item_menu_item_handler<platform::windows>::map_invoked(swipe_item_menu_item& m) {
    if (native_ == nullptr) return;
    invoked_signal_ = &m.invoked;
    if (click_token_.value != 0) {
        try { native_.Click(click_token_); } catch (...) {}
        click_token_ = {};
    }
    signal<>* target = invoked_signal_;
    try {
        click_token_ = native_.Click([target](
            ::winrt::Windows::Foundation::IInspectable const&,
            ::winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
            if (target != nullptr) target->emit();
        });
    } catch (...) {}
}

} // namespace mpapp

// ----- ADR-0013 self-registration --------------------------------------

namespace {

::winrt::Microsoft::UI::Xaml::UIElement
dispatch_swipe_item_menu_item(::mpapp::view* v) {
    if (auto* m = dynamic_cast<::mpapp::swipe_item_menu_item*>(v); m && m->has_handler()) {
        return m->handler().native();
    }
    return nullptr;
}

struct swipe_item_menu_item_registrar {
    swipe_item_menu_item_registrar() {
        ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_swipe_item_menu_item);
    }
};

[[maybe_unused]] swipe_item_menu_item_registrar _swipe_item_menu_item_reg;

} // namespace

#endif // _WIN32
