// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 slider handler implementation.

#include "mpapp/handlers/windows/slider_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

namespace mpapp {

namespace muxc  = ::winrt::Microsoft::UI::Xaml::Controls;
namespace muxcp = ::winrt::Microsoft::UI::Xaml::Controls::Primitives;

slider_handler<platform::windows>::slider_handler() {
    native_ = muxc::Slider{};
}

slider_handler<platform::windows>::~slider_handler() {
    if (native_ != nullptr && value_changed_token_.value != 0) {
        try { native_.ValueChanged(value_changed_token_); } catch (...) {}
        value_changed_token_ = {};
    }
}

void slider_handler<platform::windows>::apply_value(double v) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    native_.Value(v);
    suppress_echo_ = false;
}

void slider_handler<platform::windows>::apply_minimum(double v) {
    if (native_ != nullptr) native_.Minimum(v);
}

void slider_handler<platform::windows>::apply_maximum(double v) {
    if (native_ != nullptr) native_.Maximum(v);
}

void slider_handler<platform::windows>::map_value(slider& s) {
    bound_ = &s;
    apply_value(s.value.get());
    s.value.changed.subscribe(value_slot_, value_cb_);

    if (native_ == nullptr) return;
    if (value_changed_token_.value != 0) {
        native_.ValueChanged(value_changed_token_);
        value_changed_token_ = {};
    }
    slider* target = &s;
    auto* self = this;
    value_changed_token_ = native_.ValueChanged(
        [target, self](winrt::Windows::Foundation::IInspectable const& sender,
                       muxcp::RangeBaseValueChangedEventArgs const&) {
            if (self->suppress_echo_) return;
            auto slid = sender.as<muxc::Slider>();
            const double v = slid.Value();
            if (target->value.get() != v) {
                target->value.set(v);
            }
        });
}

void slider_handler<platform::windows>::map_minimum(slider& s) {
    apply_minimum(s.minimum.get());
    s.minimum.changed.subscribe(minimum_slot_, minimum_cb_);
}

void slider_handler<platform::windows>::map_maximum(slider& s) {
    apply_maximum(s.maximum.get());
    s.maximum.changed.subscribe(maximum_slot_, maximum_cb_);
}

} // namespace mpapp

#endif // _WIN32
