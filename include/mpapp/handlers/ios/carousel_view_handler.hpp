// SPDX-License-Identifier: Apache-2.0
// UIKit basic_carousel_view handler (blind write — compiled+run on a Mac:
// PENDING). native_ is a UIView container with one UILabel subview per
// item; only the subview at `position` is visible. Programmatic page
// switching matches the desktop handlers. Real swipe paging
// (UIPageViewController / paged UIScrollView) + peek are follow-ups.

#ifndef MPAPP_HANDLERS_IOS_CAROUSEL_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_IOS_CAROUSEL_VIEW_HANDLER_HPP

#include "../../internal/basic_carousel_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#  if TARGET_OS_IPHONE

#include <string>
#include <vector>

namespace mpapp::internal {

template <>
class carousel_view_handler<platform::ios> {
public:
    carousel_view_handler();
    ~carousel_view_handler();

    carousel_view_handler(const carousel_view_handler&)            = delete;
    carousel_view_handler& operator=(const carousel_view_handler&) = delete;
    carousel_view_handler(carousel_view_handler&&)                 = delete;
    carousel_view_handler& operator=(carousel_view_handler&&)      = delete;

    void map_items_source(basic_carousel_view& c);
    void map_position(basic_carousel_view& c);
    void map_loop(basic_carousel_view& c);
    void map_is_swipe_enabled(basic_carousel_view& c);
    void map_peek_count(basic_carousel_view& c);
    void map_gestures(basic_carousel_view& /*c*/) noexcept {}

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void rebuild_items(const std::vector<std::string>& v);
    void apply_position(int idx);

    struct items_cb_t {
        carousel_view_handler<platform::ios>* self;
        void operator()(const std::vector<std::string>&) const {
            self->rebuild_items(self->bound_ != nullptr
                                    ? self->bound_->items_source.get()
                                    : std::vector<std::string>{});
        }
    };
    struct pos_cb_t {
        carousel_view_handler<platform::ios>* self;
        void operator()(int v) const { self->apply_position(v); }
    };

    void* native_ = nullptr;   // UIView* container, retained
    basic_carousel_view* bound_ = nullptr;

    items_cb_t items_cb_{this};
    pos_cb_t   pos_cb_{this};
    signal_slot<const std::vector<std::string>&> items_slot_{};
    signal_slot<const int&>                       pos_slot_{};
};

} // namespace mpapp::internal
#  endif // TARGET_OS_IPHONE
#endif // __APPLE__
#endif // MPAPP_HANDLERS_IOS_CAROUSEL_VIEW_HANDLER_HPP
