// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 time_picker handler implementation.

#include "mpapp/handlers/windows/time_picker_handler.hpp"

#if defined(_WIN32)

#include <chrono>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "winrt_strings.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

time_picker_handler<platform::windows>::time_picker_handler() {
    native_ = muxc::TimePicker{};
}

time_picker_handler<platform::windows>::~time_picker_handler() = default;

void time_picker_handler<platform::windows>::apply_time(const time_value& v) {
    if (native_ == nullptr) return;
    try {
        // SelectedTime is an IReference<TimeSpan>; TimeSpan is std::chrono-style.
        using namespace std::chrono;
        auto ts = hours{v.hour} + minutes{v.minute};
        ::winrt::Windows::Foundation::TimeSpan span{ts};
        native_.SelectedTime(::winrt::Windows::Foundation::IReference<::winrt::Windows::Foundation::TimeSpan>{span});
    } catch (...) {}
}

void time_picker_handler<platform::windows>::apply_format(const std::string& v) {
    if (native_ == nullptr) return;
    try {
        native_.ClockIdentifier(detail::to_hstring_utf8(v.empty() ? std::string{"24HourClock"} : v));
    } catch (...) {}
}

void time_picker_handler<platform::windows>::map_time(time_picker& p) {
    apply_time(p.time.get());
    p.time.changed.subscribe(time_slot_, time_cb_);
}
void time_picker_handler<platform::windows>::map_format(time_picker& p) {
    apply_format(p.format.get());
    p.format.changed.subscribe(format_slot_, format_cb_);
}

} // namespace mpapp

#endif // _WIN32
