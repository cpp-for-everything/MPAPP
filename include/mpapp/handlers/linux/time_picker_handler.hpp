// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_time_picker handler — wraps a horizontal `GtkBox`
// containing two `GtkSpinButton`s (hour 0..23, minute 0..59). GTK4 has no
// dedicated time-basic_picker widget; the spin pair is the conventional
// alternative.

#ifndef MPAPP_HANDLERS_LINUX_TIME_PICKER_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_TIME_PICKER_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_time_picker.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class time_picker_handler<platform::linux_> {
public:
    time_picker_handler();
    ~time_picker_handler();
    time_picker_handler(const time_picker_handler&)            = delete;
    time_picker_handler& operator=(const time_picker_handler&) = delete;

    void map_time(basic_time_picker& p);
    void map_format(basic_time_picker& /*p*/) { /* GTK4 spin-pair has no format slot. */ }

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_time_picker& x);


private:
    void apply_time(const time_value& v);

    struct time_cb_t { time_picker_handler<platform::linux_>* self; void operator()(const time_value& v) const { self->apply_time(v); } };

    void* native_ = nullptr;       // GtkBox*
    void* hour_   = nullptr;       // GtkSpinButton*
    void* minute_ = nullptr;       // GtkSpinButton*

    time_cb_t                          time_cb_{this};
    signal_slot<const time_value&>     time_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_TIME_PICKER_HANDLER_HPP
