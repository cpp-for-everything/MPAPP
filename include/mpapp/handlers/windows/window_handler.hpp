// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — WinUI 3 basic_window handler.
//
// `window_handler<platform::windows>` — wraps a `winrt::Microsoft::UI::Xaml::Window`,
// propagates the cross-platform `title` / `content` / size / visibility
// changes into native calls, and forwards the native `Activated` /
// `Closed` events into the cross-platform signals.
//
// `content` is a non-owning `view*`. When the value changes, the
// handler walks the new view to find its native widget (currently via
// known subclass dispatch — basic_button / basic_label / basic_stack_layout) and
// installs it as `mux::Window::Content`.

#ifndef MPAPP_HANDLERS_WINDOWS_WINDOW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_WINDOW_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_window.hpp"

#if defined(_WIN32)

#include <string>

#include <winrt/Microsoft.UI.Xaml.h>

namespace mpapp::internal {

template <>
class window_handler<platform::windows> {
public:
    window_handler();
    ~window_handler();

    window_handler(const window_handler&)            = delete;
    window_handler& operator=(const window_handler&) = delete;
    window_handler(window_handler&&)                 = delete;
    window_handler& operator=(window_handler&&)      = delete;

    // Wires all the cross-platform properties (`title`, `content`,
    // `width`, `height`, `is_visible`) and signal forwarders in one
    // call — convenient for the application handler.
    void bind(basic_window& w);

    // Native widget access — for the application_handler to keep the
    // basic_window alive during platform teardown.
    winrt::Microsoft::UI::Xaml::Window&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Window& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_window& /*x*/) noexcept {}


private:
    void apply_title(const std::string& v);
    void apply_content(view* v);
    void apply_is_visible(bool v);

    struct title_cb_t {
        window_handler<platform::windows>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_title(v); }
    };
    struct content_cb_t {
        window_handler<platform::windows>* self = nullptr;
        void operator()(view* v) const { self->apply_content(v); }
    };
    struct visible_cb_t {
        window_handler<platform::windows>* self = nullptr;
        void operator()(bool v) const { self->apply_is_visible(v); }
    };

    winrt::Microsoft::UI::Xaml::Window native_{nullptr};
    winrt::event_token                 closed_token_{};
    basic_window*                            bound_         = nullptr;
    bool                               was_activated_ = false;

    title_cb_t                         title_cb_{this};
    content_cb_t                       content_cb_{this};
    visible_cb_t                       visible_cb_{this};
    signal_slot<const std::string&>    title_slot_{};
    signal_slot<view* const&>          content_slot_{};
    signal_slot<const bool&>           visible_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_WINDOW_HANDLER_HPP
