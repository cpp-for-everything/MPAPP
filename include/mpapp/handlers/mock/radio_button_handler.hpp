// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-platform specialisation of `radio_button_handler`.

#ifndef MPAPP_HANDLERS_MOCK_RADIO_BUTTON_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_RADIO_BUTTON_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../internal/basic_radio_button.hpp"
#include "../../signal.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class radio_button_handler<platform::mock> : public mock_handler_base {
public:
    radio_button_handler()  = default;
    ~radio_button_handler() = default;

    radio_button_handler(const radio_button_handler&)            = delete;
    radio_button_handler& operator=(const radio_button_handler&) = delete;
    radio_button_handler(radio_button_handler&&)                 = delete;
    radio_button_handler& operator=(radio_button_handler&&)      = delete;

    void map_is_checked(basic_radio_button& r) {
        record_change("is_checked", r.is_checked.get());
        r.is_checked.changed.subscribe(is_checked_slot_, is_checked_cb_);
    }

    void map_group_name(basic_radio_button& r) {
        record_change("group_name", r.group_name.get());
        r.group_name.changed.subscribe(group_name_slot_, group_name_cb_);
    }

private:
    using self_t = radio_button_handler<platform::mock>;

    mock_property_recorder<self_t, bool>        is_checked_cb_{this,
                                                               "is_checked"};
    signal_slot<const bool&>                    is_checked_slot_{};

    mock_property_recorder<self_t, std::string> group_name_cb_{this,
                                                               "group_name"};
    signal_slot<const std::string&>             group_name_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_RADIO_BUTTON_HANDLER_HPP
