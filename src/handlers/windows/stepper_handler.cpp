// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_stepper handler implementation.

#include "mpapp/handlers/windows/stepper_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

stepper_handler<platform::windows>::stepper_handler() {
    native_ = muxc::NumberBox{};
    native_.SpinButtonPlacementMode(muxc::NumberBoxSpinButtonPlacementMode::Inline);
}

stepper_handler<platform::windows>::~stepper_handler() {
    if (native_ != nullptr && value_changed_token_.value != 0) {
        try { native_.ValueChanged(value_changed_token_); } catch (...) {}
        value_changed_token_ = {};
    }
}

void stepper_handler<platform::windows>::apply_value(double v) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    native_.Value(v);
    suppress_echo_ = false;
}

void stepper_handler<platform::windows>::apply_minimum(double v) {
    if (native_ != nullptr) native_.Minimum(v);
}

void stepper_handler<platform::windows>::apply_maximum(double v) {
    if (native_ != nullptr) native_.Maximum(v);
}

void stepper_handler<platform::windows>::apply_interval(double v) {
    if (native_ != nullptr) native_.SmallChange(v);
}

void stepper_handler<platform::windows>::map_value(basic_stepper& s) {
    bound_ = &s;
    apply_value(s.value.get());
    s.value.changed.subscribe(value_slot_, value_cb_);

    if (native_ == nullptr) return;
    if (value_changed_token_.value != 0) {
        native_.ValueChanged(value_changed_token_);
        value_changed_token_ = {};
    }
    basic_stepper* target = &s;
    auto* self = this;
    value_changed_token_ = native_.ValueChanged(
        [target, self](muxc::NumberBox const& sender,
                       muxc::NumberBoxValueChangedEventArgs const&) {
            if (self->suppress_echo_) return;
            const double v = sender.Value();
            if (target->value.get() != v) {
                target->value.set(v);
            }
        });
}

void stepper_handler<platform::windows>::map_minimum(basic_stepper& s) {
    apply_minimum(s.minimum.get());
    s.minimum.changed.subscribe(minimum_slot_, minimum_cb_);
}
void stepper_handler<platform::windows>::map_maximum(basic_stepper& s) {
    apply_maximum(s.maximum.get());
    s.maximum.changed.subscribe(maximum_slot_, maximum_cb_);
}
void stepper_handler<platform::windows>::map_interval(basic_stepper& s) {
    apply_interval(s.interval.get());
    s.interval.changed.subscribe(interval_slot_, interval_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_stepper so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/internal/basic_stepper.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_stepper(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_stepper*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_stepper); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
