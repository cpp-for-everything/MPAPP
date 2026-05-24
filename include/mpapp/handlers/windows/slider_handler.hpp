// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_slider handler — wraps mux::Controls::Slider.

#ifndef MPAPP_HANDLERS_WINDOWS_SLIDER_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_SLIDER_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_slider.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.Foundation.h>

namespace mpapp::internal {

template <>
class slider_handler<platform::windows> {
public:
    slider_handler();
    ~slider_handler();

    slider_handler(const slider_handler&)            = delete;
    slider_handler& operator=(const slider_handler&) = delete;
    slider_handler(slider_handler&&)                 = delete;
    slider_handler& operator=(slider_handler&&)      = delete;

    void map_value(basic_slider& s);
    void map_minimum(basic_slider& s);
    void map_maximum(basic_slider& s);

    winrt::Microsoft::UI::Xaml::Controls::Slider&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Slider& native() const noexcept { return native_; }

private:
    void apply_value(double v);
    void apply_minimum(double v);
    void apply_maximum(double v);

    struct value_cb_t   { slider_handler<platform::windows>* self = nullptr; void operator()(double v) const { self->apply_value(v); } };
    struct minimum_cb_t { slider_handler<platform::windows>* self = nullptr; void operator()(double v) const { self->apply_minimum(v); } };
    struct maximum_cb_t { slider_handler<platform::windows>* self = nullptr; void operator()(double v) const { self->apply_maximum(v); } };

    winrt::Microsoft::UI::Xaml::Controls::Slider native_{nullptr};
    winrt::event_token                           value_changed_token_{};
    basic_slider*                                      bound_         = nullptr;
    bool                                         suppress_echo_ = false;

    value_cb_t                  value_cb_{this};
    minimum_cb_t                minimum_cb_{this};
    maximum_cb_t                maximum_cb_{this};
    signal_slot<const double&>  value_slot_{};
    signal_slot<const double&>  minimum_slot_{};
    signal_slot<const double&>  maximum_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_SLIDER_HANDLER_HPP
