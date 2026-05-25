// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_check_box handler — wraps GtkCheckButton.

#ifndef MPAPP_HANDLERS_LINUX_CHECK_BOX_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_CHECK_BOX_HANDLER_HPP

#include "../../internal/basic_check_box.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class check_box_handler<platform::linux_> {
public:
    check_box_handler();
    ~check_box_handler();

    check_box_handler(const check_box_handler&)            = delete;
    check_box_handler& operator=(const check_box_handler&) = delete;
    check_box_handler(check_box_handler&&)                 = delete;
    check_box_handler& operator=(check_box_handler&&)      = delete;

    void map_is_checked(basic_check_box& c);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_check_box& x);


private:
    void apply_is_checked(bool v);

    struct cb_t {
        check_box_handler<platform::linux_>* self = nullptr;
        void operator()(bool v) const { self->apply_is_checked(v); }
    };

    void*                    native_   = nullptr;  // GtkCheckButton*
    unsigned long            toggled_handler_id_ = 0;
    bool                     suppress_echo_ = false;
    cb_t                     cb_{this};
    signal_slot<const bool&> slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_CHECK_BOX_HANDLER_HPP
