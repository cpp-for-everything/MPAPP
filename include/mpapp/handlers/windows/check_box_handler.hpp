// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 check_box handler — wraps mux::Controls::CheckBox.

#ifndef MPAPP_HANDLERS_WINDOWS_CHECK_BOX_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_CHECK_BOX_HANDLER_HPP

#include "../../check_box.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.h>

namespace mpapp {

template <>
class check_box_handler<platform::windows> {
public:
    check_box_handler();
    ~check_box_handler();

    check_box_handler(const check_box_handler&)            = delete;
    check_box_handler& operator=(const check_box_handler&) = delete;
    check_box_handler(check_box_handler&&)                 = delete;
    check_box_handler& operator=(check_box_handler&&)      = delete;

    void map_is_checked(check_box& c);

    winrt::Microsoft::UI::Xaml::Controls::CheckBox&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::CheckBox& native() const noexcept { return native_; }

private:
    void apply_is_checked(bool v);

    struct is_checked_callback {
        check_box_handler<platform::windows>* self = nullptr;
        void operator()(bool v) const { self->apply_is_checked(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::CheckBox native_{nullptr};
    winrt::event_token                             checked_token_{};
    winrt::event_token                             unchecked_token_{};
    check_box*                                     bound_         = nullptr;
    bool                                           suppress_echo_ = false;
    is_checked_callback                            cb_{this};
    signal_slot<const bool&>                       slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_CHECK_BOX_HANDLER_HPP
