// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0003 — WinUI 3 basic_button spike.
//
// `label_handler<platform::windows>` — wraps a
// `winrt::Microsoft::UI::Xaml::Controls::TextBlock`. Single property
// (`text`) for the spike.

#ifndef MPAPP_HANDLERS_WINDOWS_LABEL_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_LABEL_HANDLER_HPP

#include "../../internal/basic_label.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <string>

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class label_handler<platform::windows> {
public:
    label_handler();
    ~label_handler();

    label_handler(const label_handler&)            = delete;
    label_handler& operator=(const label_handler&) = delete;
    label_handler(label_handler&&)                 = delete;
    label_handler& operator=(label_handler&&)      = delete;

    void map_text(basic_label& l);

    winrt::Microsoft::UI::Xaml::Controls::TextBlock&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::TextBlock& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_label& /*x*/) noexcept {}


private:
    void apply_text(std::string_view text);

    struct text_callback {
        label_handler<platform::windows>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::TextBlock native_{nullptr};
    signal_slot<const std::string&>                 text_slot_{};
    text_callback                                   text_callback_{this};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_LABEL_HANDLER_HPP
