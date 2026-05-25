// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock basic_swipe_item_menu_item handler.
//
// Records `text`, `icon_uri` via the standard `bind()` plumbing and
// `invoked` as a bare-event row whenever the signal fires.

#ifndef MPAPP_HANDLERS_MOCK_SWIPE_ITEM_MENU_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_SWIPE_ITEM_MENU_ITEM_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../internal/basic_swipe_item_menu_item.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class swipe_item_menu_item_handler<platform::mock>
    : public mock_handler_base {
public:
    swipe_item_menu_item_handler() = default;

    void map_text(basic_swipe_item_menu_item& m) {
        bind("text", m.text, binding_text_);
    }

    void map_icon_uri(basic_swipe_item_menu_item& m) {
        bind("icon_uri", m.icon_uri, binding_icon_uri_);
    }

    void map_invoked(basic_swipe_item_menu_item& m) {
        m.invoked.subscribe(invoked_slot_, invoked_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_swipe_item_menu_item& /*x*/) noexcept {}


private:
    using self_t = swipe_item_menu_item_handler<platform::mock>;

    struct invoked_cb_t {
        self_t* self;
        void operator()() const { self->record("invoked"); }
    };

    detail::property_binding<std::string>  binding_text_{};
    detail::property_binding<std::string>  binding_icon_uri_{};

    invoked_cb_t                           invoked_cb_{this};
    signal_slot<>                          invoked_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_SWIPE_ITEM_MENU_ITEM_HANDLER_HPP
