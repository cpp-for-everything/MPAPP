// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock refresh_view handler.
//
// Records `content` as a presence flag (shared_ptr<view> has no useful
// std::format spelling) plus `is_refreshing` / `refresh_color` via the
// standard `bind()` plumbing.

#ifndef MPAPP_HANDLERS_MOCK_REFRESH_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_REFRESH_VIEW_HANDLER_HPP

#include <memory>
#include <string>

#include "../../platform.hpp"
#include "../../refresh_view.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class refresh_view_handler<platform::mock>
    : public mock_handler_base {
public:
    refresh_view_handler() = default;

    // shared_ptr<view> is not std::formattable — record presence + a
    // stable string repr instead. Mirrors scroll_view / content_view.
    void map_content(refresh_view& r) {
        record("content.present", r.content.get() != nullptr);
        content_cb_ = content_cb_t{this};
        r.content.changed.subscribe(content_slot_, content_cb_);
    }

    void map_is_refreshing(refresh_view& r) {
        bind("is_refreshing", r.is_refreshing, binding_is_refreshing_);
    }

    void map_refresh_color(refresh_view& r) {
        bind("refresh_color", r.refresh_color, binding_refresh_color_);
    }

private:
    struct content_cb_t {
        refresh_view_handler<platform::mock>* self;
        void operator()(const std::shared_ptr<view>& v) const {
            self->record("content.present", v != nullptr);
        }
    };

    detail::property_binding<bool>            binding_is_refreshing_{};
    detail::property_binding<brush_ref>       binding_refresh_color_{};

    content_cb_t                              content_cb_{this};
    signal_slot<const std::shared_ptr<view>&> content_slot_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_REFRESH_VIEW_HANDLER_HPP
