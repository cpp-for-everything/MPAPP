// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-platform specialisation of `internal::button_handler`.
//
// Records every property-change emission into `mock_handler_base::calls()`.
// Tests subscribe through `map_text(b)` / `map_clicked(b)` and then mutate
// the Observable to verify the property-mapper wiring fires exactly once
// per real change (Observable's compare-on-set is what guarantees that;
// the mock proves the handler doesn't break the contract).
//
// Parameters are typed `internal::basic_button&` — the platform-agnostic
// surface — so this header doesn't pull in the wrapper class or its
// per-platform handler. The user-facing `mpapp::basic_button` wrapper inherits
// `basic_button` publicly, so it converts implicitly when an app uses the
// auto-wired wrapper.

#ifndef MPAPP_HANDLERS_MOCK_BUTTON_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_BUTTON_HANDLER_HPP

#include <string>

#include "../../internal/basic_button.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class button_handler<platform::mock> : public mock_handler_base {
public:
    button_handler()  = default;
    ~button_handler() = default;

    button_handler(const button_handler&)            = delete;
    button_handler& operator=(const button_handler&) = delete;
    button_handler(button_handler&&)                 = delete;
    button_handler& operator=(button_handler&&)      = delete;

    void map_text(basic_button& b) {
        record_change("text", b.text.get());
        b.text.changed.subscribe(text_slot_, text_cb_);
    }

    // Wires the cross-platform `clicked` signal so simulating a click via
    // `clicked_slot_.emit()` would log; tests that want to assert click
    // forwarding call `simulate_click(b)` instead — `clicked` is a signal,
    // not an Observable, so it doesn't gate change emission.
    void map_clicked(basic_button& b) {
        b.clicked.subscribe(clicked_slot_, clicked_cb_);
    }

    // Helper: simulate the platform raising the native click. The
    // cross-platform `clicked` signal forwards to subscribers (including
    // ours, which appends `"clicked"` to the call log).
    void simulate_click(basic_button& b) const { b.clicked.emit(); }

private:
    struct click_recorder {
        button_handler<platform::mock>* self = nullptr;
        void operator()() const { self->record_event("clicked"); }
    };

    mock_property_recorder<button_handler<platform::mock>, std::string> text_cb_{
        this, "text"};
    signal_slot<const std::string&> text_slot_{};

    click_recorder        clicked_cb_{this};
    signal_slot<>         clicked_slot_{};
};

} // namespace mpapp::internal

#endif // MPAPP_HANDLERS_MOCK_BUTTON_HANDLER_HPP
