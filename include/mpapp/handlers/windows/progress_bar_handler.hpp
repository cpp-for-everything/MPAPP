// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_progress_bar handler — wraps
// `mux::Controls::ProgressBar` with Minimum=0, Maximum=1, Value=progress.

#ifndef MPAPP_HANDLERS_WINDOWS_PROGRESS_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_PROGRESS_BAR_HANDLER_HPP

#include "../../platform.hpp"
#include "../../internal/basic_progress_bar.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class progress_bar_handler<platform::windows> {
public:
    progress_bar_handler();
    ~progress_bar_handler();

    progress_bar_handler(const progress_bar_handler&)            = delete;
    progress_bar_handler& operator=(const progress_bar_handler&) = delete;

    void map_progress(basic_progress_bar& p);
    void map_color(basic_progress_bar& p);
    void map_background_color(basic_progress_bar& p);

    winrt::Microsoft::UI::Xaml::Controls::ProgressBar&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::ProgressBar& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_progress_bar& /*x*/) noexcept {}


private:
    void apply_progress(double v);
    void apply_color(const brush_ref& b);
    void apply_background_color(const brush_ref& b);

    struct progress_cb_t { progress_bar_handler<platform::windows>* self; void operator()(double v) const { self->apply_progress(v); } };
    struct color_cb_t    { progress_bar_handler<platform::windows>* self; void operator()(const brush_ref& b) const { self->apply_color(b); } };
    struct bg_cb_t       { progress_bar_handler<platform::windows>* self; void operator()(const brush_ref& b) const { self->apply_background_color(b); } };

    winrt::Microsoft::UI::Xaml::Controls::ProgressBar native_{nullptr};

    progress_cb_t                 progress_cb_{this};
    color_cb_t                    color_cb_{this};
    bg_cb_t                       bg_cb_{this};
    signal_slot<const double&>    progress_slot_{};
    signal_slot<const brush_ref&> color_slot_{};
    signal_slot<const brush_ref&> bg_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_PROGRESS_BAR_HANDLER_HPP
