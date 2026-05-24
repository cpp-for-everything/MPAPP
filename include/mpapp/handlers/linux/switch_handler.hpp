// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 switch handler — wraps GtkSwitch.

#ifndef MPAPP_HANDLERS_LINUX_SWITCH_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_SWITCH_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_switch_.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class switch_handler<platform::linux_> {
public:
    switch_handler();
    ~switch_handler();

    switch_handler(const switch_handler&)            = delete;
    switch_handler& operator=(const switch_handler&) = delete;
    switch_handler(switch_handler&&)                 = delete;
    switch_handler& operator=(switch_handler&&)      = delete;

    void map_is_on(basic_switch_& s);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_is_on(bool on);

    struct is_on_callback {
        switch_handler<platform::linux_>* self = nullptr;
        void operator()(bool v) const { self->apply_is_on(v); }
    };

    void*                    native_              = nullptr;  // GtkSwitch*
    unsigned long            state_set_handler_id_ = 0;
    bool                     suppress_echo_       = false;
    is_on_callback           is_on_cb_{this};
    signal_slot<const bool&> is_on_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_SWITCH_HANDLER_HPP
