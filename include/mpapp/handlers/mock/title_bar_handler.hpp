// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock title_bar handler — records every property mapper
// invocation so unit tests can assert the exact call sequence the
// framework would have routed to a real native title-bar control.

#ifndef MPAPP_HANDLERS_MOCK_TITLE_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_TITLE_BAR_HANDLER_HPP

#include "../../platform.hpp"
#include "../../title_bar.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class title_bar_handler<platform::mock>
    : public mock_handler_base {
public:
    title_bar_handler() = default;

    void map_title(title_bar& t)    { bind("title",    t.title,    binding_title_); }
    void map_subtitle(title_bar& t) { bind("subtitle", t.subtitle, binding_subtitle_); }

private:
    detail::property_binding<std::string> binding_title_{};
    detail::property_binding<std::string> binding_subtitle_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_TITLE_BAR_HANDLER_HPP
