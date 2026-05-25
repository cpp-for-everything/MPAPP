// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock handler for `mpapp::basic_shell`.

#ifndef MPAPP_HANDLERS_MOCK_SHELL_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_SHELL_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../internal/basic_shell.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class shell_handler<platform::mock> : public mock_handler_base {
public:
    shell_handler() = default;
    ~shell_handler() = default;

    shell_handler(const shell_handler&)            = delete;
    shell_handler& operator=(const shell_handler&) = delete;
    shell_handler(shell_handler&&)                 = delete;
    shell_handler& operator=(shell_handler&&)      = delete;

    void map_current_route(basic_shell& s) {
        record_change("current_route", s.current_route.get());
        s.current_route.changed.subscribe(slot_route_, route_cb_);
    }

    void map_current_tab_index(basic_shell& s) {
        record_change("current_tab_index", s.current_tab_index.get());
        s.current_tab_index.changed.subscribe(slot_tab_, tab_cb_);
    }

    void map_is_flyout_open(basic_shell& s) {
        record_change("is_flyout_open", s.is_flyout_open.get());
        s.is_flyout_open.changed.subscribe(slot_fly_, fly_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_shell& /*x*/) noexcept {}


private:
    using self_t = shell_handler<platform::mock>;

    struct route_recorder {
        self_t* self = nullptr;
        void operator()(const std::string& v) const { self->record_change("current_route", v); }
    };
    struct tab_recorder {
        self_t* self = nullptr;
        void operator()(int v) const { self->record_change("current_tab_index", v); }
    };
    struct fly_recorder {
        self_t* self = nullptr;
        void operator()(bool v) const { self->record_change("is_flyout_open", v); }
    };

    route_recorder route_cb_{this};
    tab_recorder   tab_cb_{this};
    fly_recorder   fly_cb_{this};

    signal_slot<const std::string&> slot_route_{};
    signal_slot<const int&>         slot_tab_{};
    signal_slot<const bool&>        slot_fly_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_SHELL_HANDLER_HPP
