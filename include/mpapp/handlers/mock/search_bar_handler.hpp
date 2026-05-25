// SPDX-License-Identifier: Apache-2.0
// Mock basic_search_bar handler.

#ifndef MPAPP_HANDLERS_MOCK_SEARCH_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_SEARCH_BAR_HANDLER_HPP

#include "../../platform.hpp"
#include "../../internal/basic_search_bar.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class search_bar_handler<platform::mock>
    : public mock_handler_base {
public:
    search_bar_handler() = default;

    void map_text(basic_search_bar& s)        { bind("text",        s.text,        binding_text_); }
    void map_placeholder(basic_search_bar& s) { bind("placeholder", s.placeholder, binding_placeholder_); }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_search_bar& /*x*/) noexcept {}


private:
    detail::property_binding<std::string> binding_text_{};
    detail::property_binding<std::string> binding_placeholder_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_SEARCH_BAR_HANDLER_HPP
