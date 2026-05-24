// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_time_picker handler — wraps
// `android.widget.TimePicker` (spinner mode by default).

#ifndef MPAPP_HANDLERS_ANDROID_TIME_PICKER_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_TIME_PICKER_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_time_picker.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class time_picker_handler<platform::android> {
public:
    time_picker_handler();
    ~time_picker_handler();
    time_picker_handler(const time_picker_handler&)            = delete;
    time_picker_handler& operator=(const time_picker_handler&) = delete;

    void map_time(basic_time_picker& p);
    void map_format(basic_time_picker& /*p*/) { /* Android TimePicker has no first-class format slot. */ }

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_time(const time_value& v);

    struct time_cb_t { time_picker_handler<platform::android>* self; void operator()(const time_value& v) const { self->apply_time(v); } };

    jobject native_ = nullptr;

    time_cb_t                          time_cb_{this};
    signal_slot<const time_value&>     time_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_TIME_PICKER_HANDLER_HPP
