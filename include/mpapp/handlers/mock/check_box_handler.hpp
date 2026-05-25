// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-platform specialisation of `check_box_handler`.

#ifndef MPAPP_HANDLERS_MOCK_CHECK_BOX_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_CHECK_BOX_HANDLER_HPP

#include "../../internal/basic_check_box.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class check_box_handler<platform::mock> : public mock_handler_base {
public:
    check_box_handler()  = default;
    ~check_box_handler() = default;

    check_box_handler(const check_box_handler&)            = delete;
    check_box_handler& operator=(const check_box_handler&) = delete;
    check_box_handler(check_box_handler&&)                 = delete;
    check_box_handler& operator=(check_box_handler&&)      = delete;

    void map_is_checked(basic_check_box& c) {
        record_change("is_checked", c.is_checked.get());
        c.is_checked.changed.subscribe(is_checked_slot_, is_checked_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_check_box& /*x*/) noexcept {}


private:
    using self_t = check_box_handler<platform::mock>;

    mock_property_recorder<self_t, bool> is_checked_cb_{this, "is_checked"};
    signal_slot<const bool&>             is_checked_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_CHECK_BOX_HANDLER_HPP
