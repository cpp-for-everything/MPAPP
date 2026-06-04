// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. CommunityToolkit.Maui Toast mock handler.
//
// `toast_handler<platform::mock>` — records property mappers and
// signal subscriptions for `basic_toast` (text, duration, is_shown,
// shown/dismissed). Uses `mock_handler_base` so tests can assert the
// exact sequence of recorded changes and events via `calls_as_strings()`.

#ifndef MPAPP_HANDLERS_MOCK_TOAST_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_TOAST_HANDLER_HPP

#include <string>

#include "../../internal/basic_toast.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class toast_handler<platform::mock> : public mock_handler_base {
public:
    toast_handler()  = default;
    ~toast_handler() = default;

    toast_handler(const toast_handler&)            = delete;
    toast_handler& operator=(const toast_handler&) = delete;
    toast_handler(toast_handler&&)                 = delete;
    toast_handler& operator=(toast_handler&&)      = delete;

    // Map observable properties — record current value + subscribe for future changes.
    void map_text(basic_toast& t) {
        record_change("text", t.text.get());
        t.text.changed.subscribe(text_slot_, text_cb_);
    }

    void map_duration(basic_toast& t) {
        record_change("duration", t.duration.get());
        t.duration.changed.subscribe(duration_slot_, duration_cb_);
    }

    void map_is_shown(basic_toast& t) {
        record_change("is_shown", t.is_shown.get());
        t.is_shown.changed.subscribe(is_shown_slot_, is_shown_cb_);
    }

    // Map signals — wire `shown` / `dismissed` to append bare event rows.
    void map_shown(basic_toast& t) {
        t.shown.subscribe(shown_slot_, shown_cb_);
    }

    void map_dismissed(basic_toast& t) {
        t.dismissed.subscribe(dismissed_slot_, dismissed_cb_);
    }

private:
    // --- property recorders -----------------------------------------------
    mock_property_recorder<toast_handler<platform::mock>, std::string>
        text_cb_{this, "text"};
    signal_slot<const std::string&> text_slot_{};

    mock_property_recorder<toast_handler<platform::mock>, toast_duration>
        duration_cb_{this, "duration"};
    signal_slot<const toast_duration&> duration_slot_{};

    mock_property_recorder<toast_handler<platform::mock>, bool>
        is_shown_cb_{this, "is_shown"};
    signal_slot<const bool&> is_shown_slot_{};

    // --- event recorders --------------------------------------------------
    struct shown_recorder {
        toast_handler<platform::mock>* self = nullptr;
        void operator()() const { self->record_event("shown"); }
    };

    struct dismissed_recorder {
        toast_handler<platform::mock>* self = nullptr;
        void operator()() const { self->record_event("dismissed"); }
    };

    shown_recorder     shown_cb_{this};
    signal_slot<>      shown_slot_{};

    dismissed_recorder dismissed_cb_{this};
    signal_slot<>      dismissed_slot_{};
};

} // namespace mpapp::internal

#endif // MPAPP_HANDLERS_MOCK_TOAST_HANDLER_HPP
