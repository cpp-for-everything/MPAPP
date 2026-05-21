// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 date_picker handler implementation.

#include "mpapp/handlers/windows/date_picker_handler.hpp"

#if defined(_WIN32)

#include <chrono>
#include <ctime>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "winrt_strings.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;
namespace wf   = ::winrt::Windows::Foundation;

namespace {

wf::DateTime to_winrt(const date_value& d) {
    // Compute Unix time for (year, month, day) at midnight UTC.
    std::tm tm{};
    tm.tm_year = d.year - 1900;
    tm.tm_mon  = d.month - 1;
    tm.tm_mday = d.day;
    tm.tm_hour = 0;
    tm.tm_min  = 0;
    tm.tm_sec  = 0;
    // _mkgmtime is MSVC's UTC version of mktime.
    std::time_t epoch = _mkgmtime(&tm);
    // winrt DateTime is a time_point with 100ns ticks since 1601-01-01.
    // Use winrt::clock to construct from time_t.
    return ::winrt::clock::from_time_t(epoch);
}

} // namespace

date_picker_handler<platform::windows>::date_picker_handler() {
    native_ = muxc::CalendarDatePicker{};
}

date_picker_handler<platform::windows>::~date_picker_handler() = default;

void date_picker_handler<platform::windows>::apply_date(const date_value& v) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    try {
        native_.Date(::winrt::Windows::Foundation::IReference<wf::DateTime>{to_winrt(v)});
    } catch (...) {}
    suppress_echo_ = false;
}

void date_picker_handler<platform::windows>::apply_format(const std::string& v) {
    if (native_ == nullptr) return;
    try {
        native_.DateFormat(detail::to_hstring_utf8(v));
    } catch (...) {}
}

void date_picker_handler<platform::windows>::map_date(date_picker& p) {
    apply_date(p.date.get());
    p.date.changed.subscribe(date_slot_, date_cb_);
}
void date_picker_handler<platform::windows>::map_format(date_picker& p) {
    apply_format(p.format.get());
    p.format.changed.subscribe(format_slot_, format_cb_);
}

} // namespace mpapp


// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register date_picker so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/date_picker.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_date_picker(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::date_picker*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_date_picker); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
