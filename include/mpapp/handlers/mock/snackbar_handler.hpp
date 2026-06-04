// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Snackbar.md
//
// `snackbar_handler<platform::mock>` — records property mappers and
// event subscriptions for the `basic_snackbar` surface (text, action_text,
// duration_ms, is_shown, show/dismiss commands, action_invoked signal).

#ifndef MPAPP_HANDLERS_MOCK_SNACKBAR_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_SNACKBAR_HANDLER_HPP

#include <string>

#include "../../internal/basic_snackbar.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class snackbar_handler<platform::mock> : public mock_handler_base {
public:
    snackbar_handler()  = default;
    ~snackbar_handler() = default;

    snackbar_handler(const snackbar_handler&)            = delete;
    snackbar_handler& operator=(const snackbar_handler&) = delete;
    snackbar_handler(snackbar_handler&&)                 = delete;
    snackbar_handler& operator=(snackbar_handler&&)      = delete;

    // Property mappers -------------------------------------------------------

    void map_text(basic_snackbar& s) {
        record_change("text", s.text.get());
        s.text.changed.subscribe(text_slot_, text_cb_);
    }

    void map_action_text(basic_snackbar& s) {
        record_change("action_text", s.action_text.get());
        s.action_text.changed.subscribe(action_text_slot_, action_text_cb_);
    }

    void map_duration_ms(basic_snackbar& s) {
        record_change("duration_ms", s.duration_ms.get());
        s.duration_ms.changed.subscribe(duration_slot_, duration_cb_);
    }

    void map_is_shown(basic_snackbar& s) {
        record_change("is_shown", s.is_shown.get());
        s.is_shown.changed.subscribe(is_shown_slot_, is_shown_cb_);
    }

    // Event mappers ----------------------------------------------------------

    // Wire the `shown` signal so that when show() fires it, the handler
    // records a "shown" event entry.
    void map_shown(basic_snackbar& s) {
        s.shown.subscribe(shown_slot_, shown_cb_);
    }

    // Wire the `dismissed` signal so that when dismiss() fires it, the
    // handler records a "dismissed" event entry.
    void map_dismissed(basic_snackbar& s) {
        s.dismissed.subscribe(dismissed_slot_, dismissed_cb_);
    }

    // Wire the `action_invoked` signal so native action taps are recorded.
    void map_action_invoked(basic_snackbar& s) {
        s.action_invoked.subscribe(action_invoked_slot_, action_invoked_cb_);
    }

    // Simulation helpers (test-only) -----------------------------------------

    // Simulate the platform raising a native action-button tap.
    void simulate_action(basic_snackbar& s) const { s.action_invoked.emit(); }

private:
    // --- property recorders -------------------------------------------------

    mock_property_recorder<snackbar_handler<platform::mock>, std::string> text_cb_{
        this, "text"};
    signal_slot<const std::string&> text_slot_{};

    mock_property_recorder<snackbar_handler<platform::mock>, std::string> action_text_cb_{
        this, "action_text"};
    signal_slot<const std::string&> action_text_slot_{};

    mock_property_recorder<snackbar_handler<platform::mock>, double> duration_cb_{
        this, "duration_ms"};
    signal_slot<const double&> duration_slot_{};

    mock_property_recorder<snackbar_handler<platform::mock>, bool> is_shown_cb_{
        this, "is_shown"};
    signal_slot<const bool&> is_shown_slot_{};

    // --- event recorders ----------------------------------------------------

    struct shown_recorder {
        snackbar_handler<platform::mock>* self = nullptr;
        void operator()() const { self->record_event("shown"); }
    };

    struct dismissed_recorder {
        snackbar_handler<platform::mock>* self = nullptr;
        void operator()() const { self->record_event("dismissed"); }
    };

    struct action_invoked_recorder {
        snackbar_handler<platform::mock>* self = nullptr;
        void operator()() const { self->record_event("action_invoked"); }
    };

    shown_recorder          shown_cb_{this};
    signal_slot<>           shown_slot_{};

    dismissed_recorder      dismissed_cb_{this};
    signal_slot<>           dismissed_slot_{};

    action_invoked_recorder action_invoked_cb_{this};
    signal_slot<>           action_invoked_slot_{};
};

} // namespace mpapp::internal

#endif // MPAPP_HANDLERS_MOCK_SNACKBAR_HANDLER_HPP
