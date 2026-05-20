// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 date_picker handler — wraps `mux::Controls::CalendarDatePicker`.

#ifndef MPAPP_HANDLERS_WINDOWS_DATE_PICKER_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_DATE_PICKER_HANDLER_HPP

#include "../../date_picker.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class date_picker_handler<platform::windows> {
public:
    date_picker_handler();
    ~date_picker_handler();
    date_picker_handler(const date_picker_handler&)            = delete;
    date_picker_handler& operator=(const date_picker_handler&) = delete;

    void map_date(date_picker& p);
    void map_format(date_picker& p);

    winrt::Microsoft::UI::Xaml::Controls::CalendarDatePicker&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::CalendarDatePicker& native() const noexcept { return native_; }

private:
    void apply_date(const date_value& v);
    void apply_format(const std::string& v);

    struct date_cb_t   { date_picker_handler<platform::windows>* self; void operator()(const date_value& v) const { self->apply_date(v); } };
    struct format_cb_t { date_picker_handler<platform::windows>* self; void operator()(const std::string& v) const { self->apply_format(v); } };

    winrt::Microsoft::UI::Xaml::Controls::CalendarDatePicker native_{nullptr};
    bool suppress_echo_ = false;

    date_cb_t                          date_cb_{this};
    format_cb_t                        format_cb_{this};
    signal_slot<const date_value&>     date_slot_{};
    signal_slot<const std::string&>    format_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_DATE_PICKER_HANDLER_HPP
