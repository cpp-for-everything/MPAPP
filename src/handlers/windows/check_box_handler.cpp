// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_check_box handler implementation.

#include "mpapp/handlers/windows/check_box_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

namespace mpapp::internal {

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

void check_box_handler<platform::windows>::map_is_checked(basic_check_box& c) {
    bound_ = &c;
    apply_is_checked(c.is_checked.get());
    c.is_checked.changed.subscribe(slot_, cb_);

    if (native_ == nullptr) return;
    if (checked_token_.value   != 0) { native_.Checked(checked_token_); checked_token_ = {}; }
    if (unchecked_token_.value != 0) { native_.Unchecked(unchecked_token_); unchecked_token_ = {}; }

    basic_check_box* target = &c;
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

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_check_box so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/internal/basic_check_box.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_check_box(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_check_box*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_check_box); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
