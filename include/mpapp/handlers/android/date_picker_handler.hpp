// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_date_picker handler — wraps
// `android.widget.DatePicker` (the inline calendar/spinner variant,
// not the dialog).

#ifndef MPAPP_HANDLERS_ANDROID_DATE_PICKER_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_DATE_PICKER_HANDLER_HPP

#include "../../internal/basic_date_picker.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class date_picker_handler<platform::android> {
public:
    date_picker_handler();
    ~date_picker_handler();
    date_picker_handler(const date_picker_handler&)            = delete;
    date_picker_handler& operator=(const date_picker_handler&) = delete;

    void map_date(basic_date_picker& p);
    void map_format(basic_date_picker& /*p*/) { /* Android DatePicker has no first-class format slot. */ }

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_date_picker& /*x*/) noexcept {}


private:
    void apply_date(const date_value& v);

    struct date_cb_t { date_picker_handler<platform::android>* self; void operator()(const date_value& v) const { self->apply_date(v); } };

    jobject native_ = nullptr;

    date_cb_t                          date_cb_{this};
    signal_slot<const date_value&>     date_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_DATE_PICKER_HANDLER_HPP
