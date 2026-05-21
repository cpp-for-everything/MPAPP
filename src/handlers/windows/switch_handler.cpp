// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 switch handler implementation.

#include "mpapp/handlers/windows/switch_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

switch_handler<platform::windows>::switch_handler() {
    native_ = winrt::Microsoft::UI::Xaml::Controls::ToggleSwitch{};
}

switch_handler<platform::windows>::~switch_handler() {
    if (native_ != nullptr && toggled_token_.value != 0) {
        try { native_.Toggled(toggled_token_); } catch (...) {}
        toggled_token_ = {};
    }
}

void switch_handler<platform::windows>::apply_is_on(bool on) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    native_.IsOn(on);
    suppress_echo_ = false;
}

void switch_handler<platform::windows>::map_is_on(switch_& s) {
    bound_ = &s;
    apply_is_on(s.is_on.get());
    s.is_on.changed.subscribe(is_on_slot_, is_on_cb_);

    if (native_ == nullptr) return;
    if (toggled_token_.value != 0) {
        native_.Toggled(toggled_token_);
        toggled_token_ = {};
    }
    switch_* target = &s;
    auto* self = this;
    toggled_token_ = native_.Toggled(
        [target, self](winrt::Windows::Foundation::IInspectable const& sender,
                       winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
            if (self->suppress_echo_) return;
            auto ts = sender.as<winrt::Microsoft::UI::Xaml::Controls::ToggleSwitch>();
            const bool v = ts.IsOn();
            if (target->is_on.get() != v) {
                target->is_on.set(v);
            }
        });
}

} // namespace mpapp


// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register switch_ so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/switch_.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_switch(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::switch_*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_switch); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
