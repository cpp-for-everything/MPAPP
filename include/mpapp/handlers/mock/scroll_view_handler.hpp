// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ScrollView.md
//
// `scroll_view_handler<platform::mock>` — records property + command
// mappers for `basic_scroll_view`. The `content` property is recorded as a
// presence flag (shared_ptr<view> has no useful std::format spelling).

#ifndef MPAPP_HANDLERS_MOCK_SCROLL_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_SCROLL_VIEW_HANDLER_HPP

#include "../../platform.hpp"
#include "../../internal/basic_scroll_view.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class scroll_view_handler<platform::mock>
    : public mock_handler_base {
public:
    scroll_view_handler() = default;

    void map_orientation(basic_scroll_view& sv) {
        bind("orientation", sv.orientation, binding_orientation_);
    }

    void map_horizontal_scroll_bar_visibility(basic_scroll_view& sv) {
        bind("horizontal_scroll_bar_visibility",
             sv.horizontal_scroll_bar_visibility,
             binding_h_visibility_);
    }

    void map_vertical_scroll_bar_visibility(basic_scroll_view& sv) {
        bind("vertical_scroll_bar_visibility",
             sv.vertical_scroll_bar_visibility,
             binding_v_visibility_);
    }

    // shared_ptr<view> is not std::formattable — record presence + raw
    // pointer instead. Mirrors how a real handler discriminates "do I
    // need to attach a new child native view?" from value identity.
    void map_content(basic_scroll_view& sv) {
        record("content.present", sv.content.get() != nullptr);
        content_callback_ = content_cb{this};
        sv.content.changed.subscribe(content_slot_, content_callback_);
    }

    // Command: scroll-to. Tests invoke this directly to record the request.
    void map_scroll_to(basic_scroll_view& /*sv*/, const scroll_to_request& req) {
        record("scroll_to.x", req.x);
        record("scroll_to.y", req.y);
        record("scroll_to.animated", req.animated);
    }

private:
    detail::property_binding<scroll_orientation>    binding_orientation_{};
    detail::property_binding<scroll_bar_visibility> binding_h_visibility_{};
    detail::property_binding<scroll_bar_visibility> binding_v_visibility_{};

    struct content_cb {
        scroll_view_handler* self;
        void operator()(const std::shared_ptr<view>& v) const {
            self->record("content.present", v != nullptr);
        }
    };
    signal_slot<const std::shared_ptr<view>&> content_slot_{};
    content_cb                                content_callback_{this};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_SCROLL_VIEW_HANDLER_HPP
