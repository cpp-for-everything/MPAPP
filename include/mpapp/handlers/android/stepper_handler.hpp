// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android stepper handler — wraps android.widget.NumberPicker.
//
// NumberPicker uses int values. The handler scales the cross-platform
// double [minimum, maximum, interval] surface onto an int range via
// `step_index = round((value - minimum) / interval)` so the user gets
// discrete steps consistent with mpapp::stepper::interval.

#ifndef MPAPP_HANDLERS_ANDROID_STEPPER_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_STEPPER_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../stepper.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class stepper_handler<platform::android> {
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

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

    void on_native_value_changed(int step_index);

private:
    void apply_range_and_value();

    struct value_cb_t    { stepper_handler<platform::android>* self = nullptr; void operator()(double) const { self->apply_range_and_value(); } };
    struct minimum_cb_t  { stepper_handler<platform::android>* self = nullptr; void operator()(double) const { self->apply_range_and_value(); } };
    struct maximum_cb_t  { stepper_handler<platform::android>* self = nullptr; void operator()(double) const { self->apply_range_and_value(); } };
    struct interval_cb_t { stepper_handler<platform::android>* self = nullptr; void operator()(double) const { self->apply_range_and_value(); } };

    jobject     native_   = nullptr;  // global ref to NumberPicker
    jobject     listener_ = nullptr;
    stepper*    bound_    = nullptr;
    bool        suppress_echo_ = false;

    value_cb_t                   value_cb_{this};
    minimum_cb_t                 minimum_cb_{this};
    maximum_cb_t                 maximum_cb_{this};
    interval_cb_t                interval_cb_{this};
    signal_slot<const double&>   value_slot_{};
    signal_slot<const double&>   minimum_slot_{};
    signal_slot<const double&>   maximum_slot_{};
    signal_slot<const double&>   interval_slot_{};
};

void android_stepper_dispatch_value(stepper_handler<platform::android>* h, int step_index);

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_STEPPER_HANDLER_HPP
