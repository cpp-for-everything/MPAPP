// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 time_picker handler — wraps `mux::Controls::TimePicker`.

#ifndef MPAPP_HANDLERS_WINDOWS_TIME_PICKER_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_TIME_PICKER_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../time_picker.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class time_picker_handler<platform::windows> {
public:
    time_picker_handler();
    ~time_picker_handler();
    time_picker_handler(const time_picker_handler&)            = delete;
    time_picker_handler& operator=(const time_picker_handler&) = delete;

    void map_time(time_picker& p);
    void map_format(time_picker& p);

    winrt::Microsoft::UI::Xaml::Controls::TimePicker&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::TimePicker& native() const noexcept { return native_; }

private:
    void apply_time(const time_value& v);
    void apply_format(const std::string& v);

    struct time_cb_t   { time_picker_handler<platform::windows>* self; void operator()(const time_value& v) const { self->apply_time(v); } };
    struct format_cb_t { time_picker_handler<platform::windows>* self; void operator()(const std::string& v) const { self->apply_format(v); } };

    winrt::Microsoft::UI::Xaml::Controls::TimePicker native_{nullptr};

    time_cb_t                          time_cb_{this};
    format_cb_t                        format_cb_{this};
    signal_slot<const time_value&>     time_slot_{};
    signal_slot<const std::string&>    format_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_TIME_PICKER_HANDLER_HPP
