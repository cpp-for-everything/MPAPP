// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_swipe_view handler.
//
// `android.widget.FrameLayout` host for the wrapped content. The
// `androidx.viewpager2.widget.ViewPager2`-driven swipe is deferred to a
// follow-up batch (it needs an adapter contract that doesn't exist yet
// in M-04b). The `left_items` / `right_items` vectors are tracked via
// the registry but the host renders only the content child for now.

#ifndef MPAPP_HANDLERS_ANDROID_SWIPE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_SWIPE_VIEW_HANDLER_HPP

#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_swipe_view.hpp"
#include "../../view.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class swipe_view_handler<platform::android> {
public:
    swipe_view_handler();
    ~swipe_view_handler();

    swipe_view_handler(const swipe_view_handler&)            = delete;
    swipe_view_handler& operator=(const swipe_view_handler&) = delete;
    swipe_view_handler(swipe_view_handler&&)                 = delete;
    swipe_view_handler& operator=(swipe_view_handler&&)      = delete;

    void map_content(basic_swipe_view& sv);
    void map_left_items(basic_swipe_view& sv);
    void map_right_items(basic_swipe_view& sv);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_swipe_view& /*x*/) noexcept {}


private:
    void apply_content(view* v);
    void apply_left_items(const std::vector<view*>& items);
    void apply_right_items(const std::vector<view*>& items);

    struct content_cb_t {
        swipe_view_handler<platform::android>* self;
        void operator()(view* v) const { self->apply_content(v); }
    };
    struct left_cb_t {
        swipe_view_handler<platform::android>* self;
        void operator()(const std::vector<view*>& v) const { self->apply_left_items(v); }
    };
    struct right_cb_t {
        swipe_view_handler<platform::android>* self;
        void operator()(const std::vector<view*>& v) const { self->apply_right_items(v); }
    };

    jobject native_        = nullptr; // FrameLayout host (global ref)
    jobject current_child_ = nullptr; // content child (global ref)

    content_cb_t                                content_cb_{this};
    left_cb_t                                   left_cb_{this};
    right_cb_t                                  right_cb_{this};
    signal_slot<view* const&>                   content_slot_{};
    signal_slot<const std::vector<view*>&>      left_slot_{};
    signal_slot<const std::vector<view*>&>      right_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_SWIPE_VIEW_HANDLER_HPP
