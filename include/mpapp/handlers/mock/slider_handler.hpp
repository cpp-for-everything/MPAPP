// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-platform specialisation of `slider_handler`.

#ifndef MPAPP_HANDLERS_MOCK_SLIDER_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_SLIDER_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_slider.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class slider_handler<platform::mock> : public mock_handler_base {
public:
    slider_handler()  = default;
    ~slider_handler() = default;

    slider_handler(const slider_handler&)            = delete;
    slider_handler& operator=(const slider_handler&) = delete;
    slider_handler(slider_handler&&)                 = delete;
    slider_handler& operator=(slider_handler&&)      = delete;

    void map_value(basic_slider& s) {
        record_change("value", s.value.get());
        s.value.changed.subscribe(value_slot_, value_cb_);
    }

    void map_minimum(basic_slider& s) {
        record_change("minimum", s.minimum.get());
        s.minimum.changed.subscribe(minimum_slot_, minimum_cb_);
    }

    void map_maximum(basic_slider& s) {
        record_change("maximum", s.maximum.get());
        s.maximum.changed.subscribe(maximum_slot_, maximum_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_slider& /*x*/) noexcept {}


private:
    using self_t = slider_handler<platform::mock>;

    mock_property_recorder<self_t, double> value_cb_{this, "value"};
    signal_slot<const double&>             value_slot_{};

    mock_property_recorder<self_t, double> minimum_cb_{this, "minimum"};
    signal_slot<const double&>             minimum_slot_{};

    mock_property_recorder<self_t, double> maximum_cb_{this, "maximum"};
    signal_slot<const double&>             maximum_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_SLIDER_HANDLER_HPP
