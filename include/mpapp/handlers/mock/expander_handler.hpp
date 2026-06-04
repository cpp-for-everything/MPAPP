// SPDX-License-Identifier: Apache-2.0
// Mock-platform specialisation of `internal::expander_handler`.
//
// Records every property-change emission into `mock_handler_base::calls()`.
// Tests subscribe through map_* methods and then mutate Observables or emit
// signals to verify the property-mapper and event-mapper wiring.

#ifndef MPAPP_HANDLERS_MOCK_EXPANDER_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_EXPANDER_HANDLER_HPP

#include "../../internal/basic_expander.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class expander_handler<platform::mock> : public mock_handler_base {
public:
    expander_handler()  = default;
    ~expander_handler() = default;

    expander_handler(const expander_handler&)            = delete;
    expander_handler& operator=(const expander_handler&) = delete;
    expander_handler(expander_handler&&)                 = delete;
    expander_handler& operator=(expander_handler&&)      = delete;

    // Record the initial is_expanded value and subscribe to future changes.
    void map_is_expanded(basic_expander& e) {
        bind("is_expanded", e.is_expanded, binding_is_expanded_);
    }

    // Record the initial direction value and subscribe to future changes.
    void map_direction(basic_expander& e) {
        bind("direction", e.direction, binding_direction_);
    }

    // Record header slot presence.
    void map_header(basic_expander& e) {
        record("header", e.header() != nullptr ? std::string_view{"set"}
                                               : std::string_view{"null"});
    }

    // Record content slot presence.
    void map_content(basic_expander& e) {
        record("content", e.content() != nullptr ? std::string_view{"set"}
                                                 : std::string_view{"null"});
    }

    // Wire the expanded signal so simulating expansion logs "expanded".
    void map_expanded(basic_expander& e) {
        e.expanded.subscribe(expanded_slot_, expanded_cb_);
    }

    // Wire the collapsed signal so simulating collapse logs "collapsed".
    void map_collapsed(basic_expander& e) {
        e.collapsed.subscribe(collapsed_slot_, collapsed_cb_);
    }

    // RFC-0003 stub: per-platform real gesture wire-up is pending the
    // platform's real-handler task. No-op today so the wrapper ctor's
    // unconditional `embedded_handler_.map_gestures(*this)` links.
    void map_gestures(basic_expander& /*e*/) noexcept {}

private:
    detail::property_binding<bool>             binding_is_expanded_{};
    detail::property_binding<expand_direction> binding_direction_{};

    struct expanded_cb_t {
        expander_handler<platform::mock>* self = nullptr;
        void operator()() const { self->record_event("expanded"); }
    };

    struct collapsed_cb_t {
        expander_handler<platform::mock>* self = nullptr;
        void operator()() const { self->record_event("collapsed"); }
    };

    expanded_cb_t  expanded_cb_{this};
    signal_slot<>  expanded_slot_{};

    collapsed_cb_t collapsed_cb_{this};
    signal_slot<>  collapsed_slot_{};
};

} // namespace mpapp::internal

#endif // MPAPP_HANDLERS_MOCK_EXPANDER_HANDLER_HPP
