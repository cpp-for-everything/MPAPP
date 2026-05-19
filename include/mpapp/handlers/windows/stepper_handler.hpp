// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 stepper handler — wraps mux::Controls::NumberBox
// with the inline +/- spin buttons enabled.

#ifndef MPAPP_HANDLERS_WINDOWS_STEPPER_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_STEPPER_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../stepper.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.h>

namespace mpapp {

template <>
class stepper_handler<platform::windows> {
public:
    stepper_handler();
    ~stepper_handler();

    stepper_handler(const stepper_handler&)            = delete;
    stepper_handler& operator=(const stepper_handler&) = delete;
    stepper_handler(stepper_handler&&)                 = delete;
    stepper_handler& operator=(stepper_handler&&)      = delete;

    void map_value(stepper& s);
    void map_minimum(stepper& s);
    void map_maximum(stepper& s);
    void map_interval(stepper& s);

    winrt::Microsoft::UI::Xaml::Controls::NumberBox&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::NumberBox& native() const noexcept { return native_; }

private:
    void apply_value(double v);
    void apply_minimum(double v);
    void apply_maximum(double v);
    void apply_interval(double v);

    struct value_cb_t    { stepper_handler<platform::windows>* self = nullptr; void operator()(double v) const { self->apply_value(v); } };
    struct minimum_cb_t  { stepper_handler<platform::windows>* self = nullptr; void operator()(double v) const { self->apply_minimum(v); } };
    struct maximum_cb_t  { stepper_handler<platform::windows>* self = nullptr; void operator()(double v) const { self->apply_maximum(v); } };
    struct interval_cb_t { stepper_handler<platform::windows>* self = nullptr; void operator()(double v) const { self->apply_interval(v); } };

    winrt::Microsoft::UI::Xaml::Controls::NumberBox native_{nullptr};
    winrt::event_token                              value_changed_token_{};
    stepper*                                        bound_         = nullptr;
    bool                                            suppress_echo_ = false;

    value_cb_t                   value_cb_{this};
    minimum_cb_t                 minimum_cb_{this};
    maximum_cb_t                 maximum_cb_{this};
    interval_cb_t                interval_cb_{this};
    signal_slot<const double&>   value_slot_{};
    signal_slot<const double&>   minimum_slot_{};
    signal_slot<const double&>   maximum_slot_{};
    signal_slot<const double&>   interval_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_STEPPER_HANDLER_HPP
