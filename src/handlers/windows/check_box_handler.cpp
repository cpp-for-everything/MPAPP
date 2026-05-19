// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 check_box handler implementation.

#include "mpapp/handlers/windows/check_box_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

namespace mpapp {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

check_box_handler<platform::windows>::check_box_handler() {
    native_ = muxc::CheckBox{};
}

check_box_handler<platform::windows>::~check_box_handler() {
    if (native_ == nullptr) return;
    try {
        if (checked_token_.value   != 0) native_.Checked(checked_token_);
        if (unchecked_token_.value != 0) native_.Unchecked(unchecked_token_);
    } catch (...) {}
    checked_token_ = {};
    unchecked_token_ = {};
}

void check_box_handler<platform::windows>::apply_is_checked(bool v) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    native_.IsChecked(winrt::Windows::Foundation::IReference<bool>{v});
    suppress_echo_ = false;
}

void check_box_handler<platform::windows>::map_is_checked(check_box& c) {
    bound_ = &c;
    apply_is_checked(c.is_checked.get());
    c.is_checked.changed.subscribe(slot_, cb_);

    if (native_ == nullptr) return;
    if (checked_token_.value   != 0) { native_.Checked(checked_token_); checked_token_ = {}; }
    if (unchecked_token_.value != 0) { native_.Unchecked(unchecked_token_); unchecked_token_ = {}; }

    check_box* target = &c;
    auto* self = this;
    auto handler_set = [target, self](bool value) {
        if (self->suppress_echo_) return;
        if (target->is_checked.get() != value) {
            target->is_checked.set(value);
        }
    };
    checked_token_   = native_.Checked(
        [handler_set](winrt::Windows::Foundation::IInspectable const&,
                      mux::RoutedEventArgs const&) { handler_set(true); });
    unchecked_token_ = native_.Unchecked(
        [handler_set](winrt::Windows::Foundation::IInspectable const&,
                      mux::RoutedEventArgs const&) { handler_set(false); });
}

} // namespace mpapp

#endif // _WIN32
