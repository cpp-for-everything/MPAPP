// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock swipe_view handler.
//
// Records the three property mappers:
//   - content.present   (bool — view* nullness flag)
//   - left_items.count  (vector size)
//   - right_items.count (vector size)
//
// View* is not std::formattable on its own, but the bool / size_t projections
// are. Mirrors the projection-on-bind shape used by page (content.present)
// and refresh_view (content.present).

#ifndef MPAPP_HANDLERS_MOCK_SWIPE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_SWIPE_VIEW_HANDLER_HPP

#include <cstddef>
#include <vector>

#include "../../platform.hpp"
#include "../../swipe_view.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class swipe_view_handler<platform::mock>
    : public mock_handler_base {
public:
    swipe_view_handler() = default;

    void map_content(swipe_view& sv) {
        record("content.present", sv.content.get() != nullptr);
        sv.content.changed.subscribe(content_slot_, content_cb_);
    }

    void map_left_items(swipe_view& sv) {
        record("left_items.count", static_cast<std::size_t>(sv.left_items.get().size()));
        sv.left_items.changed.subscribe(left_slot_, left_cb_);
    }

    void map_right_items(swipe_view& sv) {
        record("right_items.count", static_cast<std::size_t>(sv.right_items.get().size()));
        sv.right_items.changed.subscribe(right_slot_, right_cb_);
    }

private:
    using self_t = swipe_view_handler<platform::mock>;

    struct content_cb_t {
        self_t* self;
        void operator()(view* v) const { self->record("content.present", v != nullptr); }
    };
    struct left_cb_t {
        self_t* self;
        void operator()(const std::vector<view*>& v) const {
            self->record("left_items.count", static_cast<std::size_t>(v.size()));
        }
    };
    struct right_cb_t {
        self_t* self;
        void operator()(const std::vector<view*>& v) const {
            self->record("right_items.count", static_cast<std::size_t>(v.size()));
        }
    };

    content_cb_t                                  content_cb_{this};
    left_cb_t                                     left_cb_{this};
    right_cb_t                                    right_cb_{this};
    signal_slot<view* const&>                     content_slot_{};
    signal_slot<const std::vector<view*>&>        left_slot_{};
    signal_slot<const std::vector<view*>&>        right_slot_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_SWIPE_VIEW_HANDLER_HPP
