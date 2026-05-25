// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Window.md
//
// `window_handler<platform::mock>` — records the title / content /
// visibility / size mapper invocations. Tests assert against the call
// log to verify the framework drives the handler correctly when the
// user mutates the cross-platform Observables.

#ifndef MPAPP_HANDLERS_MOCK_WINDOW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_WINDOW_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../internal/basic_window.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class window_handler<platform::mock> : public mock_handler_base {
public:
    window_handler() = default;
    ~window_handler() = default;

    window_handler(const window_handler&)            = delete;
    window_handler& operator=(const window_handler&) = delete;
    window_handler(window_handler&&)                 = delete;
    window_handler& operator=(window_handler&&)      = delete;

    void map_title(basic_window& w) {
        record_change("title", w.title.get());
        w.title.changed.subscribe(title_slot_, title_cb_);
    }

    void map_content(basic_window& w) {
        // Record whether content is set (true/false); a real handler
        // attaches the native widget pointed to by the underlying view*.
        record_change("content.present", w.content.get() != nullptr);
        w.content.changed.subscribe(content_slot_, content_cb_);
    }

    void map_width(basic_window& w) {
        record_change("width", w.width.get());
        w.width.changed.subscribe(width_slot_, width_cb_);
    }

    void map_height(basic_window& w) {
        record_change("height", w.height.get());
        w.height.changed.subscribe(height_slot_, height_cb_);
    }

    void map_is_visible(basic_window& w) {
        record_change("is_visible", w.is_visible.get());
        w.is_visible.changed.subscribe(visible_slot_, visible_cb_);
    }

    // Simulate the platform raising the activated signal (e.g. WinUI's
    // `Window::Activated` event firing). Tests use this to verify the
    // user-side `basic_window::activated.subscribe(...)` plumbing.
    void simulate_activated(basic_window& w) const {
        w.activated.emit();
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_window& /*x*/) noexcept {}


private:
    using self_t = window_handler<platform::mock>;

    struct content_recorder {
        self_t* self = nullptr;
        void operator()(view* v) const { self->record_change("content.present", v != nullptr); }
    };

    mock_property_recorder<self_t, std::string> title_cb_{this, "title"};
    signal_slot<const std::string&>             title_slot_{};

    content_recorder                            content_cb_{this};
    signal_slot<view* const&>                   content_slot_{};

    mock_property_recorder<self_t, int>         width_cb_{this, "width"};
    signal_slot<const int&>                     width_slot_{};

    mock_property_recorder<self_t, int>         height_cb_{this, "height"};
    signal_slot<const int&>                     height_slot_{};

    mock_property_recorder<self_t, bool>        visible_cb_{this, "is_visible"};
    signal_slot<const bool&>                    visible_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_WINDOW_HANDLER_HPP
