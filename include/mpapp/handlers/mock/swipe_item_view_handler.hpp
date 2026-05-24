// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock basic_swipe_item_view handler.
//
// One property mapper: `content.present` (bool projection of view*).

#ifndef MPAPP_HANDLERS_MOCK_SWIPE_ITEM_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_SWIPE_ITEM_VIEW_HANDLER_HPP

#include "../../platform.hpp"
#include "../../internal/basic_swipe_item_view.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class swipe_item_view_handler<platform::mock>
    : public mock_handler_base {
public:
    swipe_item_view_handler() = default;

    void map_content(basic_swipe_item_view& iv) {
        record("content.present", iv.content.get() != nullptr);
        iv.content.changed.subscribe(content_slot_, content_cb_);
    }

private:
    using self_t = swipe_item_view_handler<platform::mock>;

    struct content_cb_t {
        self_t* self;
        void operator()(view* v) const { self->record("content.present", v != nullptr); }
    };

    content_cb_t                   content_cb_{this};
    signal_slot<view* const&>      content_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_SWIPE_ITEM_VIEW_HANDLER_HPP
