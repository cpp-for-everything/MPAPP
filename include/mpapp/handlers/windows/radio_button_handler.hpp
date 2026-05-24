// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_radio_button handler — wraps mux::Controls::RadioButton.
// Native grouping is via the GroupName property; setting two RadioButtons to
// the same GroupName makes WinUI auto-uncheck siblings.

#ifndef MPAPP_HANDLERS_WINDOWS_RADIO_BUTTON_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_RADIO_BUTTON_HANDLER_HPP

#include "../../platform.hpp"
#include "../../internal/basic_radio_button.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <string>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.h>

namespace mpapp::internal {

template <>
class radio_button_handler<platform::windows> {
public:
    radio_button_handler();
    ~radio_button_handler();

    radio_button_handler(const radio_button_handler&)            = delete;
    radio_button_handler& operator=(const radio_button_handler&) = delete;
    radio_button_handler(radio_button_handler&&)                 = delete;
    radio_button_handler& operator=(radio_button_handler&&)      = delete;

    void map_is_checked(basic_radio_button& r);
    void map_group_name(basic_radio_button& r);

    winrt::Microsoft::UI::Xaml::Controls::RadioButton&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::RadioButton& native() const noexcept { return native_; }

private:
    void apply_is_checked(bool v);
    void apply_group_name(const std::string& v);

    struct is_checked_callback {
        radio_button_handler<platform::windows>* self = nullptr;
        void operator()(bool v) const { self->apply_is_checked(v); }
    };
    struct group_name_callback {
        radio_button_handler<platform::windows>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_group_name(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::RadioButton native_{nullptr};
    winrt::event_token                                 checked_token_{};
    winrt::event_token                                 unchecked_token_{};
    basic_radio_button*                                      bound_         = nullptr;
    bool                                               suppress_echo_ = false;

    is_checked_callback  is_checked_cb_{this};
    group_name_callback  group_name_cb_{this};
    signal_slot<const bool&>        is_checked_slot_{};
    signal_slot<const std::string&> group_name_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_RADIO_BUTTON_HANDLER_HPP
