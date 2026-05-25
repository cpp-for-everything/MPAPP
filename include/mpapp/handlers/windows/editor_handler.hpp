// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_editor handler — wraps mux::Controls::TextBox
// configured for multi-line input (AcceptsReturn=true, TextWrapping=Wrap).

#ifndef MPAPP_HANDLERS_WINDOWS_EDITOR_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_EDITOR_HANDLER_HPP

#include "../../internal/basic_editor.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <string>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.h>

namespace mpapp::internal {

template <>
class editor_handler<platform::windows> {
public:
    editor_handler();
    ~editor_handler();

    editor_handler(const editor_handler&)            = delete;
    editor_handler& operator=(const editor_handler&) = delete;
    editor_handler(editor_handler&&)                 = delete;
    editor_handler& operator=(editor_handler&&)      = delete;

    void map_text(basic_editor& e);
    void map_placeholder(basic_editor& e);
    void map_is_read_only(basic_editor& e);

    winrt::Microsoft::UI::Xaml::Controls::TextBox&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::TextBox& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_editor& /*x*/) noexcept {}


private:
    void apply_text(std::string_view text);
    void apply_placeholder(std::string_view text);
    void apply_is_read_only(bool ro);

    struct text_cb_t {
        editor_handler<platform::windows>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct placeholder_cb_t {
        editor_handler<platform::windows>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_placeholder(v); }
    };
    struct readonly_cb_t {
        editor_handler<platform::windows>* self = nullptr;
        void operator()(bool v) const { self->apply_is_read_only(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::TextBox native_{nullptr};
    winrt::event_token                            text_changed_token_{};
    basic_editor*                                       bound_         = nullptr;
    bool                                          suppress_echo_ = false;
    text_cb_t                                     text_cb_{this};
    placeholder_cb_t                              placeholder_cb_{this};
    readonly_cb_t                                 readonly_cb_{this};
    signal_slot<const std::string&>               text_slot_{};
    signal_slot<const std::string&>               placeholder_slot_{};
    signal_slot<const bool&>                      readonly_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_EDITOR_HANDLER_HPP
