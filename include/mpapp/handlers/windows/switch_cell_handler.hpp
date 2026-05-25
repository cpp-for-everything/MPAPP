// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_switch_cell handler — Border wrapping a 2-column Grid:
// left column = basic_label TextBlock, right column = ToggleSwitch bound to
// the cell's `on` observable. ToggleSwitch.Toggled echoes user flips
// back through the Observable (suppress_echo_ guards reentrancy).

#ifndef MPAPP_HANDLERS_WINDOWS_SWITCH_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_SWITCH_CELL_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_switch_cell.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class switch_cell_handler<platform::windows> {
public:
    switch_cell_handler();
    ~switch_cell_handler();

    switch_cell_handler(const switch_cell_handler&)            = delete;
    switch_cell_handler& operator=(const switch_cell_handler&) = delete;
    switch_cell_handler(switch_cell_handler&&)                 = delete;
    switch_cell_handler& operator=(switch_cell_handler&&)      = delete;

    void map_text(basic_switch_cell& c);
    void map_on(basic_switch_cell& c);

    winrt::Microsoft::UI::Xaml::Controls::Border&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Border& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_switch_cell& /*x*/) noexcept {}


private:
    void apply_text(const std::string& v);
    void apply_on(bool v);

    struct text_cb_t {
        switch_cell_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct on_cb_t {
        switch_cell_handler<platform::windows>* self;
        void operator()(bool v) const { self->apply_on(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Border       native_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::Grid         grid_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::TextBlock    text_block_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::ToggleSwitch toggle_{nullptr};

    winrt::event_token toggled_token_{};
    basic_switch_cell*       bound_         = nullptr;
    bool               suppress_echo_ = false;

    text_cb_t                       text_cb_{this};
    on_cb_t                         on_cb_{this};
    signal_slot<const std::string&> text_slot_{};
    signal_slot<const bool&>        on_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_SWITCH_CELL_HANDLER_HPP
