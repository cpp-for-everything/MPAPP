// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_swipe_item_view handler.
//
// `android.widget.FrameLayout` content host. Renders inline (no
// gesture-revealed pill) until the parent `basic_swipe_view` wires the
// gesture plumbing.

#ifndef MPAPP_HANDLERS_ANDROID_SWIPE_ITEM_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_SWIPE_ITEM_VIEW_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_swipe_item_view.hpp"
#include "../../view.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class swipe_item_view_handler<platform::android> {
public:
    swipe_item_view_handler();
    ~swipe_item_view_handler();

    swipe_item_view_handler(const swipe_item_view_handler&)            = delete;
    swipe_item_view_handler& operator=(const swipe_item_view_handler&) = delete;
    swipe_item_view_handler(swipe_item_view_handler&&)                 = delete;
    swipe_item_view_handler& operator=(swipe_item_view_handler&&)      = delete;

    void map_content(basic_swipe_item_view& iv);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_content(view* v);

    struct content_cb_t {
        swipe_item_view_handler<platform::android>* self;
        void operator()(view* v) const { self->apply_content(v); }
    };

    jobject native_        = nullptr; // FrameLayout (global ref)
    jobject current_child_ = nullptr; // wrapped content (global ref)

    content_cb_t              content_cb_{this};
    signal_slot<view* const&> content_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_SWIPE_ITEM_VIEW_HANDLER_HPP
