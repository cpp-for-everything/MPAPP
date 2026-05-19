// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 follow-up — WinUI 3 entry handler.
//
// `entry_handler<platform::windows>` wraps a
// `winrt::Microsoft::UI::Xaml::Controls::TextBox` and maps the
// cross-platform `mpapp::entry` Observables onto the native widget.
// Reverse binding (user typing into the TextBox → `entry::text.set`)
// is wired via the native `TextChanged` event.

#ifndef MPAPP_HANDLERS_WINDOWS_ENTRY_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_ENTRY_HANDLER_HPP

#include "../../entry.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <string>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.h>

namespace mpapp {

template <>
class entry_handler<platform::windows> {
public:
    entry_handler();
    ~entry_handler();

    entry_handler(const entry_handler&)            = delete;
    entry_handler& operator=(const entry_handler&) = delete;
    entry_handler(entry_handler&&)                 = delete;
    entry_handler& operator=(entry_handler&&)      = delete;

    void map_text(entry& e);
    void map_placeholder(entry& e);
    void map_is_read_only(entry& e);

    winrt::Microsoft::UI::Xaml::Controls::TextBox&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::TextBox& native() const noexcept { return native_; }

private:
    void apply_text(std::string_view text);
    void apply_placeholder(std::string_view text);
    void apply_is_read_only(bool ro);

    struct text_callback {
        entry_handler<platform::windows>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
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
    entry*                                        bound_         = nullptr;
    bool                                          suppress_echo_ = false;
    text_callback                                 text_cb_{this};
    placeholder_callback                          placeholder_cb_{this};
    readonly_callback                             readonly_cb_{this};
    signal_slot<const std::string&>               text_slot_{};
    signal_slot<const std::string&>               placeholder_slot_{};
    signal_slot<const bool&>                      readonly_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_ENTRY_HANDLER_HPP
