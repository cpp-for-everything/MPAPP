// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock basic_title_bar handler — records every property mapper
// invocation so unit tests can assert the exact call sequence the
// framework would have routed to a real native title-bar control.

#ifndef MPAPP_HANDLERS_MOCK_TITLE_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_TITLE_BAR_HANDLER_HPP

#include "../../platform.hpp"
#include "../../internal/basic_title_bar.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class title_bar_handler<platform::mock>
    : public mock_handler_base {
public:
    title_bar_handler() = default;

    void map_title(basic_title_bar& t)    { bind("title",    t.title,    binding_title_); }
    void map_subtitle(basic_title_bar& t) { bind("subtitle", t.subtitle, binding_subtitle_); }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_title_bar& /*x*/) noexcept {}


private:
    detail::property_binding<std::string> binding_title_{};
    detail::property_binding<std::string> binding_subtitle_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_TITLE_BAR_HANDLER_HPP
