// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_time_picker handler implementation.

#include "mpapp/handlers/windows/time_picker_handler.hpp"

#if defined(_WIN32)

#include <chrono>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "winrt_strings.hpp"

namespace mpapp::internal {

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

void time_picker_handler<platform::windows>::map_time(basic_time_picker& p) {
    apply_time(p.time.get());
    p.time.changed.subscribe(time_slot_, time_cb_);
}
void time_picker_handler<platform::windows>::map_format(basic_time_picker& p) {
    apply_format(p.format.get());
    p.format.changed.subscribe(format_slot_, format_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_time_picker so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/internal/basic_time_picker.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_time_picker(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_time_picker*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_time_picker); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
