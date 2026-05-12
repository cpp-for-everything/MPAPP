// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0003 — WinUI 3 button spike.
//
// `button_handler<platform::windows>` — wraps a
// `winrt::Microsoft::UI::Xaml::Controls::Button`, propagates property
// changes from the cross-platform `button`'s Observables to the native
// widget, and forwards the native `Click` event back into the
// cross-platform `clicked` signal.

#ifndef MPAPP_HANDLERS_WINDOWS_BUTTON_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_BUTTON_HANDLER_HPP

#include "../../button.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <string>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.h>

namespace mpapp {

template <>
class button_handler<platform::windows> {
public:
    button_handler();
    ~button_handler();

    button_handler(const button_handler&)            = delete;
    button_handler& operator=(const button_handler&) = delete;
    button_handler(button_handler&&)                 = delete;
    button_handler& operator=(button_handler&&)      = delete;

    // Property-mapper hook. Pushes `b.text.get()` into the native widget
    // and wires the change-signal so subsequent sets propagate.
    void map_text(button& b);

    // Wires the native `Click` event into `b.clicked`. Idempotent.
    void map_clicked(button& b);

    // Native widget access — for the host to add to a Window content tree.
    winrt::Microsoft::UI::Xaml::Controls::Button&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Button& native() const noexcept { return native_; }

private:
    void apply_text(std::string_view text);

    // Tiny stable callable stored as a member so its address can be
    // handed to `signal::subscribe`, which holds it by reference. No
    // heap, no std::function.
    struct text_callback {
        button_handler<platform::windows>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Button native_{nullptr};
    winrt::event_token                           click_token_{};
    signal_slot<const std::string&>              text_slot_{};
    text_callback                                text_callback_{this};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_BUTTON_HANDLER_HPP
