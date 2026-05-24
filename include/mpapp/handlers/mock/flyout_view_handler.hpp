// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock basic_flyout_view handler.
//
// Records `flyout` and `detail` as presence flags (shared_ptr<view>
// has no useful std::format spelling) plus `is_presented` via the
// standard `bind()` plumbing.

#ifndef MPAPP_HANDLERS_MOCK_FLYOUT_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_FLYOUT_VIEW_HANDLER_HPP

#include <memory>

#include "../../internal/basic_flyout_view.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class flyout_view_handler<platform::mock>
    : public mock_handler_base {
public:
    flyout_view_handler() = default;

    // shared_ptr<view> is not std::formattable — record presence + a
    // stable string repr instead. Mirrors basic_refresh_view's mock handler.
    void map_flyout(basic_flyout_view& f) {
        record("flyout.present", f.flyout.get() != nullptr);
        flyout_cb_ = flyout_cb_t{this};
        f.flyout.changed.subscribe(flyout_slot_, flyout_cb_);
    }

    void map_detail(basic_flyout_view& f) {
        record("detail.present", f.detail.get() != nullptr);
        detail_cb_ = detail_cb_t{this};
        f.detail.changed.subscribe(detail_slot_, detail_cb_);
    }

    void map_is_presented(basic_flyout_view& f) {
        bind("is_presented", f.is_presented, binding_is_presented_);
    }

private:
    struct flyout_cb_t {
        flyout_view_handler<platform::mock>* self;
        void operator()(const std::shared_ptr<view>& v) const {
            self->record("flyout.present", v != nullptr);
        }
    };
    struct detail_cb_t {
        flyout_view_handler<platform::mock>* self;
        void operator()(const std::shared_ptr<view>& v) const {
            self->record("detail.present", v != nullptr);
        }
    };

    detail::property_binding<bool>            binding_is_presented_{};

    flyout_cb_t                               flyout_cb_{this};
    detail_cb_t                               detail_cb_{this};
    signal_slot<const std::shared_ptr<view>&> flyout_slot_{};
    signal_slot<const std::shared_ptr<view>&> detail_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_FLYOUT_VIEW_HANDLER_HPP
