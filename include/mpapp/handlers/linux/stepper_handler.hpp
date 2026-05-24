// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_stepper handler — wraps GtkSpinButton.

#ifndef MPAPP_HANDLERS_LINUX_STEPPER_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_STEPPER_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_stepper.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class stepper_handler<platform::linux_> {
public:
    stepper_handler();
    ~stepper_handler();

    stepper_handler(const stepper_handler&)            = delete;
    stepper_handler& operator=(const stepper_handler&) = delete;
    stepper_handler(stepper_handler&&)                 = delete;
    stepper_handler& operator=(stepper_handler&&)      = delete;

    void map_value(basic_stepper& s);
    void map_minimum(basic_stepper& s);
    void map_maximum(basic_stepper& s);
    void map_interval(basic_stepper& s);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_value(double v);
    void apply_minimum(double v);
    void apply_maximum(double v);
    void apply_interval(double v);

    struct value_cb_t    { stepper_handler<platform::linux_>* self = nullptr; void operator()(double v) const { self->apply_value(v); } };
    struct minimum_cb_t  { stepper_handler<platform::linux_>* self = nullptr; void operator()(double v) const { self->apply_minimum(v); } };
    struct maximum_cb_t  { stepper_handler<platform::linux_>* self = nullptr; void operator()(double v) const { self->apply_maximum(v); } };
    struct interval_cb_t { stepper_handler<platform::linux_>* self = nullptr; void operator()(double v) const { self->apply_interval(v); } };

    void*           native_              = nullptr;  // GtkSpinButton*
    unsigned long   value_changed_handler_id_ = 0;
    bool            suppress_echo_       = false;

    value_cb_t                   value_cb_{this};
    minimum_cb_t                 minimum_cb_{this};
    maximum_cb_t                 maximum_cb_{this};
    interval_cb_t                interval_cb_{this};
    signal_slot<const double&>   value_slot_{};
    signal_slot<const double&>   minimum_slot_{};
    signal_slot<const double&>   maximum_slot_{};
    signal_slot<const double&>   interval_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_STEPPER_HANDLER_HPP
