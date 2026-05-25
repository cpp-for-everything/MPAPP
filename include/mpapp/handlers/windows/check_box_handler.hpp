// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_check_box handler — wraps mux::Controls::CheckBox.

#ifndef MPAPP_HANDLERS_WINDOWS_CHECK_BOX_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_CHECK_BOX_HANDLER_HPP

#include "../../internal/basic_check_box.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.h>

namespace mpapp::internal {

template <>
class check_box_handler<platform::windows> {
public:
    check_box_handler();
    ~check_box_handler();

    check_box_handler(const check_box_handler&)            = delete;
    check_box_handler& operator=(const check_box_handler&) = delete;
    check_box_handler(check_box_handler&&)                 = delete;
    check_box_handler& operator=(check_box_handler&&)      = delete;

    void map_is_checked(basic_check_box& c);

    winrt::Microsoft::UI::Xaml::Controls::CheckBox&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::CheckBox& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_check_box& /*x*/) noexcept {}


private:
    void apply_is_checked(bool v);

    struct is_checked_callback {
        check_box_handler<platform::windows>* self = nullptr;
        void operator()(bool v) const { self->apply_is_checked(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::CheckBox native_{nullptr};
    winrt::event_token                             checked_token_{};
    winrt::event_token                             unchecked_token_{};
    basic_check_box*                                     bound_         = nullptr;
    bool                                           suppress_echo_ = false;
    is_checked_callback                            cb_{this};
    signal_slot<const bool&>                       slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_CHECK_BOX_HANDLER_HPP
