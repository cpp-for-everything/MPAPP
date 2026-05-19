// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 radio_button handler — wraps GtkCheckButton with a
// group set via `gtk_check_button_set_group`. Two radio_buttons sharing
// the same group_name string get auto-linked into the same GTK group at
// bind time.

#ifndef MPAPP_HANDLERS_LINUX_RADIO_BUTTON_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_RADIO_BUTTON_HANDLER_HPP

#include "../../platform.hpp"
#include "../../radio_button.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <string>

namespace mpapp {

template <>
class radio_button_handler<platform::linux_> {
public:
    radio_button_handler();
    ~radio_button_handler();

    radio_button_handler(const radio_button_handler&)            = delete;
    radio_button_handler& operator=(const radio_button_handler&) = delete;
    radio_button_handler(radio_button_handler&&)                 = delete;
    radio_button_handler& operator=(radio_button_handler&&)      = delete;

    void map_is_checked(radio_button& r);
    void map_group_name(radio_button& r);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_is_checked(bool v);
    void apply_group_name(const std::string& v);

    struct is_checked_cb_t {
        radio_button_handler<platform::linux_>* self = nullptr;
        void operator()(bool v) const { self->apply_is_checked(v); }
    };
    struct group_name_cb_t {
        radio_button_handler<platform::linux_>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_group_name(v); }
    };

    void*                    native_   = nullptr;  // GtkCheckButton*
    unsigned long            toggled_handler_id_ = 0;
    bool                     suppress_echo_ = false;

    is_checked_cb_t                  is_checked_cb_{this};
    group_name_cb_t                  group_name_cb_{this};
    signal_slot<const bool&>         is_checked_slot_{};
    signal_slot<const std::string&>  group_name_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_RADIO_BUTTON_HANDLER_HPP
