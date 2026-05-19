// SPDX-License-Identifier: Apache-2.0
// Mock search_bar handler.

#ifndef MPAPP_HANDLERS_MOCK_SEARCH_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_SEARCH_BAR_HANDLER_HPP

#include "../../platform.hpp"
#include "../../search_bar.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class search_bar_handler<platform::mock>
    : public mock_handler_base {
public:
    search_bar_handler() = default;

    void map_text(search_bar& s)        { bind("text",        s.text,        binding_text_); }
    void map_placeholder(search_bar& s) { bind("placeholder", s.placeholder, binding_placeholder_); }

private:
    detail::property_binding<std::string> binding_text_{};
    detail::property_binding<std::string> binding_placeholder_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_SEARCH_BAR_HANDLER_HPP
