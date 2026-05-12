// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-platform specialisation of `stepper_handler`.

#ifndef MPAPP_HANDLERS_MOCK_STEPPER_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_STEPPER_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../stepper.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class stepper_handler<platform::mock> : public mock_handler_base {
public:
    stepper_handler()  = default;
    ~stepper_handler() = default;

    stepper_handler(const stepper_handler&)            = delete;
    stepper_handler& operator=(const stepper_handler&) = delete;
    stepper_handler(stepper_handler&&)                 = delete;
    stepper_handler& operator=(stepper_handler&&)      = delete;

    void map_value(stepper& s) {
        record_change("value", s.value.get());
        s.value.changed.subscribe(value_slot_, value_cb_);
    }

    void map_minimum(stepper& s) {
        record_change("minimum", s.minimum.get());
        s.minimum.changed.subscribe(minimum_slot_, minimum_cb_);
    }

    void map_maximum(stepper& s) {
        record_change("maximum", s.maximum.get());
        s.maximum.changed.subscribe(maximum_slot_, maximum_cb_);
    }

    void map_interval(stepper& s) {
        record_change("interval", s.interval.get());
        s.interval.changed.subscribe(interval_slot_, interval_cb_);
    }

private:
    using self_t = stepper_handler<platform::mock>;

    mock_property_recorder<self_t, double> value_cb_{this, "value"};
    signal_slot<const double&>             value_slot_{};

    mock_property_recorder<self_t, double> minimum_cb_{this, "minimum"};
    signal_slot<const double&>             minimum_slot_{};

    mock_property_recorder<self_t, double> maximum_cb_{this, "maximum"};
    signal_slot<const double&>             maximum_slot_{};

    mock_property_recorder<self_t, double> interval_cb_{this, "interval"};
    signal_slot<const double&>             interval_slot_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_STEPPER_HANDLER_HPP
