// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/DrawingView.md
//
// `drawing_view_handler<platform::mock>` — records property mappers and
// command invocations for `basic_drawing_view` (default_line_color,
// default_line_width, is_multi_line_mode, clear, add_line).

#ifndef MPAPP_HANDLERS_MOCK_DRAWING_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_DRAWING_VIEW_HANDLER_HPP

#include <string>

#include "../../internal/basic_drawing_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class drawing_view_handler<platform::mock>
    : public mock_handler_base {
public:
    drawing_view_handler()  = default;
    ~drawing_view_handler() = default;

    drawing_view_handler(const drawing_view_handler&)            = delete;
    drawing_view_handler& operator=(const drawing_view_handler&) = delete;
    drawing_view_handler(drawing_view_handler&&)                 = delete;
    drawing_view_handler& operator=(drawing_view_handler&&)      = delete;

    // Wire up Observable<brush_ref> default_line_color.
    void map_default_line_color(basic_drawing_view& v) {
        bind("default_line_color", v.default_line_color, binding_color_);
    }

    // Wire up Observable<double> default_line_width.
    void map_default_line_width(basic_drawing_view& v) {
        bind("default_line_width", v.default_line_width, binding_width_);
    }

    // Wire up Observable<bool> is_multi_line_mode.
    void map_is_multi_line_mode(basic_drawing_view& v) {
        bind("is_multi_line_mode", v.is_multi_line_mode, binding_multi_);
    }

    // Record a "clear" command invocation — no value payload.
    void map_clear(basic_drawing_view& v) {
        // Subscribe to a custom "clear requested" event; for the mock
        // surface the handler records the *current* state synchronously at
        // wire-up time (mirrors other command mappers), then the test can
        // call simulate_clear() to drive further records.
        (void)v;
        record("clear");
    }

    // Record an "add_line" command invocation with the line payload.
    void map_add_line(basic_drawing_view& /*v*/,
                      const mpapp::drawing_line& line) {
        record("add_line", line);
    }

    // Subscribe to drawing_started signal.
    void map_drawing_started(basic_drawing_view& v) {
        v.drawing_started.subscribe(started_slot_, started_cb_);
    }

    // Subscribe to drawing_line_completed signal.
    void map_drawing_line_completed(basic_drawing_view& v) {
        v.drawing_line_completed.subscribe(completed_slot_, completed_cb_);
    }

    // Test helpers: simulate the platform raising the native signals.
    void simulate_drawing_started(basic_drawing_view& v) const {
        v.drawing_started.emit();
    }

    void simulate_drawing_line_completed(basic_drawing_view& v,
                                         const mpapp::drawing_line& line) const {
        v.drawing_line_completed.emit(line);
    }

private:
    struct started_recorder {
        drawing_view_handler<platform::mock>* self = nullptr;
        void operator()() const { self->record_event("drawing_started"); }
    };

    struct completed_recorder {
        drawing_view_handler<platform::mock>* self = nullptr;
        void operator()(const mpapp::drawing_line& line) const {
            self->record_change("drawing_line_completed", line);
        }
    };

    detail::property_binding<mpapp::brush_ref> binding_color_{};
    detail::property_binding<double>           binding_width_{};
    detail::property_binding<bool>             binding_multi_{};

    started_recorder                               started_cb_{this};
    signal_slot<>                                  started_slot_{};

    completed_recorder                             completed_cb_{this};
    signal_slot<const mpapp::drawing_line&>        completed_slot_{};
};

} // namespace mpapp::internal

#endif // MPAPP_HANDLERS_MOCK_DRAWING_VIEW_HANDLER_HPP
