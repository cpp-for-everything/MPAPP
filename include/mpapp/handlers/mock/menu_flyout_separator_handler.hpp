// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock menu_flyout_separator handler.
//
// The separator type has no observable properties. The mock handler
// exists so the layout-test plumbing has a uniform shape across the
// four menu_flyout-family widgets. It records a single `"separator"`
// event when the framework asks for a bind-time snapshot.

#ifndef MPAPP_HANDLERS_MOCK_MENU_FLYOUT_SEPARATOR_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_MENU_FLYOUT_SEPARATOR_HANDLER_HPP

#include "../../menu_flyout_separator.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class menu_flyout_separator_handler<platform::mock>
    : public mock_handler_base {
public:
    menu_flyout_separator_handler() = default;

    // No observable properties on the separator — this mapper records a
    // single bare-event entry so tests can verify the handler was
    // wired. Real handlers don't expose this mapper.
    void map_bind(menu_flyout_separator& /*s*/) {
        record_event("separator");
    }
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_MENU_FLYOUT_SEPARATOR_HANDLER_HPP
