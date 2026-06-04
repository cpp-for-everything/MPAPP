// SPDX-License-Identifier: Apache-2.0
// Mock handler for `mpapp::internal::basic_two_pane_view`.

#ifndef MPAPP_HANDLERS_MOCK_TWO_PANE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_TWO_PANE_VIEW_HANDLER_HPP

#include <memory>

#include "../../internal/basic_two_pane_view.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class two_pane_view_handler<platform::mock>
    : public mock_handler_base {
public:
    two_pane_view_handler() = default;

    // Records the current pane1 presence, then subscribes to future changes.
    void map_pane1(basic_two_pane_view& v) {
        record("pane1.present", v.pane1() != nullptr);
        pane1_cb_ = pane1_recorder{this};
        v.pane1_changed.subscribe(slot_pane1_, pane1_cb_);
    }

    // Records the current pane2 presence, then subscribes to future changes.
    void map_pane2(basic_two_pane_view& v) {
        record("pane2.present", v.pane2() != nullptr);
        pane2_cb_ = pane2_recorder{this};
        v.pane2_changed.subscribe(slot_pane2_, pane2_cb_);
    }

    void map_mode(basic_two_pane_view& v) {
        bind("mode", v.mode, binding_mode_);
    }

    void map_panel_priority(basic_two_pane_view& v) {
        bind("panel_priority", v.panel_priority, binding_priority_);
    }

    void map_min_wide_mode_width(basic_two_pane_view& v) {
        bind("min_wide_mode_width", v.min_wide_mode_width, binding_wide_width_);
    }

    void map_min_tall_mode_height(basic_two_pane_view& v) {
        bind("min_tall_mode_height", v.min_tall_mode_height, binding_tall_height_);
    }

    // RFC-0003 stub: per-platform real gesture wire-up is pending the
    // platform's real-handler task. No-op today so the wrapper ctor's
    // unconditional `embedded_handler_.map_gestures(*this);` links.
    void map_gestures(basic_two_pane_view& /*x*/) noexcept {}

private:
    struct pane1_recorder {
        two_pane_view_handler<platform::mock>* self = nullptr;
        void operator()(const std::shared_ptr<view>& p) const {
            self->record("pane1.present", p != nullptr);
        }
    };
    struct pane2_recorder {
        two_pane_view_handler<platform::mock>* self = nullptr;
        void operator()(const std::shared_ptr<view>& p) const {
            self->record("pane2.present", p != nullptr);
        }
    };

    pane1_recorder pane1_cb_{this};
    pane2_recorder pane2_cb_{this};

    signal_slot<const std::shared_ptr<view>&> slot_pane1_{};
    signal_slot<const std::shared_ptr<view>&> slot_pane2_{};

    detail::property_binding<two_pane_mode>     binding_mode_{};
    detail::property_binding<two_pane_priority> binding_priority_{};
    detail::property_binding<double>            binding_wide_width_{};
    detail::property_binding<double>            binding_tall_height_{};
};

} // namespace mpapp::internal

#endif // MPAPP_HANDLERS_MOCK_TWO_PANE_VIEW_HANDLER_HPP
