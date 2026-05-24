// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 switch handler — wraps mux::Controls::ToggleSwitch.

#ifndef MPAPP_HANDLERS_WINDOWS_SWITCH_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_SWITCH_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_switch_.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.h>

namespace mpapp::internal {

template <>
class switch_handler<platform::windows> {
public:
    switch_handler();
    ~switch_handler();

    switch_handler(const switch_handler&)            = delete;
    switch_handler& operator=(const switch_handler&) = delete;
    switch_handler(switch_handler&&)                 = delete;
    switch_handler& operator=(switch_handler&&)      = delete;

    void map_is_on(basic_switch_& s);

    winrt::Microsoft::UI::Xaml::Controls::ToggleSwitch&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::ToggleSwitch& native() const noexcept { return native_; }

private:
    void apply_is_on(bool on);

    struct is_on_callback {
        switch_handler<platform::windows>* self = nullptr;
        void operator()(bool v) const { self->apply_is_on(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::ToggleSwitch native_{nullptr};
    winrt::event_token                                 toggled_token_{};
    basic_switch_*                                           bound_         = nullptr;
    bool                                               suppress_echo_ = false;
    is_on_callback                                     is_on_cb_{this};
    signal_slot<const bool&>                           is_on_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_SWITCH_HANDLER_HPP
