// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-platform specialisation of `switch_handler`.

#ifndef MPAPP_HANDLERS_MOCK_SWITCH_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_SWITCH_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_switch_.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class switch_handler<platform::mock> : public mock_handler_base {
public:
    switch_handler()  = default;
    ~switch_handler() = default;

    switch_handler(const switch_handler&)            = delete;
    switch_handler& operator=(const switch_handler&) = delete;
    switch_handler(switch_handler&&)                 = delete;
    switch_handler& operator=(switch_handler&&)      = delete;

    void map_is_on(basic_switch_& s) {
        record_change("is_on", s.is_on.get());
        s.is_on.changed.subscribe(is_on_slot_, is_on_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_switch_& /*x*/) noexcept {}


private:
    using self_t = switch_handler<platform::mock>;

    mock_property_recorder<self_t, bool> is_on_cb_{this, "is_on"};
    signal_slot<const bool&>             is_on_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_SWITCH_HANDLER_HPP
