// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 activity_indicator handler — wraps
// `mux::Controls::ProgressRing`.

#ifndef MPAPP_HANDLERS_WINDOWS_ACTIVITY_INDICATOR_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_ACTIVITY_INDICATOR_HANDLER_HPP

#include "../../activity_indicator.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class activity_indicator_handler<platform::windows> {
public:
    activity_indicator_handler();
    ~activity_indicator_handler();

    activity_indicator_handler(const activity_indicator_handler&)            = delete;
    activity_indicator_handler& operator=(const activity_indicator_handler&) = delete;
    activity_indicator_handler(activity_indicator_handler&&)                 = delete;
    activity_indicator_handler& operator=(activity_indicator_handler&&)      = delete;

    void map_is_running(activity_indicator& a);
    void map_color(activity_indicator& a);

    winrt::Microsoft::UI::Xaml::Controls::ProgressRing&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::ProgressRing& native() const noexcept { return native_; }

private:
    void apply_is_running(bool v);
    void apply_color(const brush_ref& b);

    struct is_running_cb_t { activity_indicator_handler<platform::windows>* self; void operator()(bool v) const { self->apply_is_running(v); } };
    struct color_cb_t      { activity_indicator_handler<platform::windows>* self; void operator()(const brush_ref& b) const { self->apply_color(b); } };

    winrt::Microsoft::UI::Xaml::Controls::ProgressRing native_{nullptr};

    is_running_cb_t                       is_running_cb_{this};
    color_cb_t                            color_cb_{this};
    signal_slot<const bool&>              is_running_slot_{};
    signal_slot<const brush_ref&>         color_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_ACTIVITY_INDICATOR_HANDLER_HPP
