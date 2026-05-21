// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 radio_button handler implementation.

#include "mpapp/handlers/windows/radio_button_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

#include "winrt_strings.hpp"

namespace mpapp {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

radio_button_handler<platform::windows>::radio_button_handler() {
    native_ = muxc::RadioButton{};
}

radio_button_handler<platform::windows>::~radio_button_handler() {
    if (native_ == nullptr) return;
    try {
        if (checked_token_.value   != 0) native_.Checked(checked_token_);
        if (unchecked_token_.value != 0) native_.Unchecked(unchecked_token_);
    } catch (...) {}
    checked_token_ = {};
    unchecked_token_ = {};
}

void radio_button_handler<platform::windows>::apply_is_checked(bool v) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    native_.IsChecked(winrt::Windows::Foundation::IReference<bool>{v});
    suppress_echo_ = false;
}

void radio_button_handler<platform::windows>::apply_group_name(const std::string& v) {
    if (native_ == nullptr) return;
    native_.GroupName(detail::to_hstring_utf8(v));
}

void radio_button_handler<platform::windows>::map_is_checked(radio_button& r) {
    bound_ = &r;
    apply_is_checked(r.is_checked.get());
    r.is_checked.changed.subscribe(is_checked_slot_, is_checked_cb_);

    if (native_ == nullptr) return;
    if (checked_token_.value   != 0) { native_.Checked(checked_token_); checked_token_ = {}; }
    if (unchecked_token_.value != 0) { native_.Unchecked(unchecked_token_); unchecked_token_ = {}; }

    radio_button* target = &r;
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

void radio_button_handler<platform::windows>::map_group_name(radio_button& r) {
    apply_group_name(r.group_name.get());
    r.group_name.changed.subscribe(group_name_slot_, group_name_cb_);
}

} // namespace mpapp


// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register radio_button so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/radio_button.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_radio_button(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::radio_button*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_radio_button); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
