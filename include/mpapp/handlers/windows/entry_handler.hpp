// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 follow-up — WinUI 3 basic_entry handler.
//
// `entry_handler<platform::windows>` wraps a
// `winrt::Microsoft::UI::Xaml::Controls::TextBox` and maps the
// cross-platform `mpapp::basic_entry` Observables onto the native widget.
// Reverse binding (user typing into the TextBox → `basic_entry::text.set`)
// is wired via the native `TextChanged` event.

#ifndef MPAPP_HANDLERS_WINDOWS_ENTRY_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_ENTRY_HANDLER_HPP

#include "../../internal/basic_entry.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <string>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.h>

namespace mpapp::internal {

template <>
class entry_handler<platform::windows> {
public:
    entry_handler();
    ~entry_handler();

    entry_handler(const entry_handler&)            = delete;
    entry_handler& operator=(const entry_handler&) = delete;
    entry_handler(entry_handler&&)                 = delete;
    entry_handler& operator=(entry_handler&&)      = delete;

    void map_text(basic_entry& e);
    void map_placeholder(basic_entry& e);
    void map_is_read_only(basic_entry& e);
    void map_semantics(basic_entry& e);   // AutomationProperties.Name (a11y)

    winrt::Microsoft::UI::Xaml::Controls::TextBox&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::TextBox& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_entry& /*x*/) noexcept {}


private:
    void apply_text(std::string_view text);
    void apply_placeholder(std::string_view text);
    void apply_is_read_only(bool ro);
    void apply_semantics(std::string_view desc);

    struct text_callback {
        entry_handler<platform::windows>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct sem_callback {
        entry_handler<platform::windows>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_semantics(v); }
    };
    struct placeholder_callback {
        entry_handler<platform::windows>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_placeholder(v); }
    };
    struct readonly_callback {
        entry_handler<platform::windows>* self = nullptr;
        void operator()(bool v) const { self->apply_is_read_only(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::TextBox native_{nullptr};
    winrt::event_token                            text_changed_token_{};
    basic_entry*                                        bound_         = nullptr;
    bool                                          suppress_echo_ = false;
    text_callback                                 text_cb_{this};
    placeholder_callback                          placeholder_cb_{this};
    readonly_callback                             readonly_cb_{this};
    sem_callback                                  sem_cb_{this};
    signal_slot<const std::string&>               text_slot_{};
    signal_slot<const std::string&>               placeholder_slot_{};
    signal_slot<const bool&>                      readonly_slot_{};
    signal_slot<const std::string&>               sem_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_ENTRY_HANDLER_HPP
