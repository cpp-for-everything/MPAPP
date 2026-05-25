// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_entry_cell handler — Border wrapping a 2-column Grid:
// leading basic_label TextBlock + trailing TextBox bound to `text`. KeyDown
// Enter emits `completed`. Keyboard kind maps to InputScope.

#ifndef MPAPP_HANDLERS_WINDOWS_ENTRY_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_ENTRY_CELL_HANDLER_HPP

#include <string>

#include "../../internal/basic_entry_cell.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>

namespace mpapp::internal {

template <>
class entry_cell_handler<platform::windows> {
public:
    entry_cell_handler();
    ~entry_cell_handler();

    entry_cell_handler(const entry_cell_handler&)            = delete;
    entry_cell_handler& operator=(const entry_cell_handler&) = delete;
    entry_cell_handler(entry_cell_handler&&)                 = delete;
    entry_cell_handler& operator=(entry_cell_handler&&)      = delete;

    void map_label(basic_entry_cell& c);
    void map_text(basic_entry_cell& c);
    void map_placeholder(basic_entry_cell& c);
    void map_keyboard(basic_entry_cell& c);

    winrt::Microsoft::UI::Xaml::Controls::Border&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Border& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_entry_cell& /*x*/) noexcept {}


private:
    void apply_label(const std::string& v);
    void apply_text(const std::string& v);
    void apply_placeholder(const std::string& v);
    void apply_keyboard(keyboard_kind v);

    struct label_cb_t {
        entry_cell_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_label(v); }
    };
    struct text_cb_t {
        entry_cell_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct placeholder_cb_t {
        entry_cell_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_placeholder(v); }
    };
    struct keyboard_cb_t {
        entry_cell_handler<platform::windows>* self;
        void operator()(keyboard_kind v) const { self->apply_keyboard(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Border    native_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::Grid      grid_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::TextBlock label_block_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::TextBox   text_box_{nullptr};

    winrt::event_token text_changed_token_{};
    winrt::event_token key_down_token_{};
    basic_entry_cell*        bound_         = nullptr;
    bool               suppress_echo_ = false;

    label_cb_t                          label_cb_{this};
    text_cb_t                           text_cb_{this};
    placeholder_cb_t                    placeholder_cb_{this};
    keyboard_cb_t                       keyboard_cb_{this};
    signal_slot<const std::string&>     label_slot_{};
    signal_slot<const std::string&>     text_slot_{};
    signal_slot<const std::string&>     placeholder_slot_{};
    signal_slot<const keyboard_kind&>   keyboard_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_ENTRY_CELL_HANDLER_HPP
