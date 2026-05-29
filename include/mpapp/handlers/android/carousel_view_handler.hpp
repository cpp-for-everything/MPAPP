// SPDX-License-Identifier: Apache-2.0
// Android basic_carousel_view handler. Wraps `android.widget.ViewFlipper`
// — a framework-built-in ViewAnimator (no androidx dependency) that shows
// one child at a time and switches pages via setDisplayedChild(int). That
// maps cleanly to CarouselView's Position. items_source becomes one
// TextView child per item. loop/clamp is applied in
// basic_carousel_view::scroll_to; peek_count shows one page (no peek);
// swipe-to-page (GestureDetector) is a follow-up — Android is programmatic
// + tap-gesture-driven in v1 (a ViewPager2 upgrade would add fling paging
// but pulls in androidx + a RecyclerView adapter).

#ifndef MPAPP_HANDLERS_ANDROID_CAROUSEL_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_CAROUSEL_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../internal/basic_carousel_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class carousel_view_handler<platform::android> {
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
    void map_gestures(basic_carousel_view& c);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void rebuild_items(const std::vector<std::string>& v);
    void apply_position(int idx);

    struct items_cb_t {
        carousel_view_handler<platform::android>* self;
        void operator()(const std::vector<std::string>&) const {
            self->rebuild_items(self->bound_ != nullptr
                                    ? self->bound_->items_source.get()
                                    : std::vector<std::string>{});
        }
    };
    struct pos_cb_t {
        carousel_view_handler<platform::android>* self;
        void operator()(int v) const { self->apply_position(v); }
    };

    jobject native_ = nullptr;   // global ref to ViewFlipper
    basic_carousel_view* bound_ = nullptr;

    items_cb_t items_cb_{this};
    pos_cb_t   pos_cb_{this};
    signal_slot<const std::vector<std::string>&> items_slot_{};
    signal_slot<const int&>                       pos_slot_{};
};

} // namespace mpapp::internal

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_CAROUSEL_VIEW_HANDLER_HPP
