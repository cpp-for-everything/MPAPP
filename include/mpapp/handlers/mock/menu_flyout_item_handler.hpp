// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock basic_menu_flyout_item handler.
//
// Records `text` and `is_enabled` via the standard `bind()` plumbing.
// Tests that need to verify the click signal subscribe to
// `mfi.clicked` directly — the mock handler doesn't drive the signal,
// real handlers do.

#ifndef MPAPP_HANDLERS_MOCK_MENU_FLYOUT_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_MENU_FLYOUT_ITEM_HANDLER_HPP

#include <string>

#include "../../internal/basic_menu_flyout_item.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class menu_flyout_item_handler<platform::mock>
    : public mock_handler_base {
public:
    menu_flyout_item_handler() = default;

    void map_text(basic_menu_flyout_item& i)       { bind("text",       i.text,       binding_text_); }
    void map_is_enabled(basic_menu_flyout_item& i) { bind("is_enabled", i.is_enabled, binding_is_enabled_); }

private:
    detail::property_binding<std::string> binding_text_{};
    detail::property_binding<bool>        binding_is_enabled_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_MENU_FLYOUT_ITEM_HANDLER_HPP
