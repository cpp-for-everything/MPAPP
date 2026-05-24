// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_date_picker handler — wraps `GtkCalendar`.

#ifndef MPAPP_HANDLERS_LINUX_DATE_PICKER_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_DATE_PICKER_HANDLER_HPP

#include "../../internal/basic_date_picker.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class date_picker_handler<platform::linux_> {
public:
    date_picker_handler();
    ~date_picker_handler();
    date_picker_handler(const date_picker_handler&)            = delete;
    date_picker_handler& operator=(const date_picker_handler&) = delete;

    void map_date(basic_date_picker& p);
    void map_format(basic_date_picker& /*p*/) { /* GtkCalendar has no first-class format slot. */ }

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_date(const date_value& v);

    struct date_cb_t { date_picker_handler<platform::linux_>* self; void operator()(const date_value& v) const { self->apply_date(v); } };

    void* native_ = nullptr;  // GtkCalendar*

    date_cb_t                          date_cb_{this};
    signal_slot<const date_value&>     date_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_DATE_PICKER_HANDLER_HPP
