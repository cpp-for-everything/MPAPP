// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_scroll_view handler.
//
// Wraps android.widget.ScrollView (vertical) / HorizontalScrollView
// depending on orientation. Single-child container — matches the
// cross-platform contract.

#ifndef MPAPP_HANDLERS_ANDROID_SCROLL_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_SCROLL_VIEW_HANDLER_HPP

#include <memory>

#include "../../platform.hpp"
#include "../../internal/basic_scroll_view.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class scroll_view_handler<platform::android> {
public:
    scroll_view_handler();
    ~scroll_view_handler();

    scroll_view_handler(const scroll_view_handler&)            = delete;
    scroll_view_handler& operator=(const scroll_view_handler&) = delete;
    scroll_view_handler(scroll_view_handler&&)                 = delete;
    scroll_view_handler& operator=(scroll_view_handler&&)      = delete;

    void map_content(basic_scroll_view& s);
    void map_orientation(basic_scroll_view& s);

    void bind_content(basic_scroll_view& s, view& child);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_scroll_view& /*x*/) noexcept {}


private:
    void apply_content(const std::shared_ptr<view>& v);
    void apply_orientation(scroll_orientation o);  // no-op; vertical default

    struct content_cb_t {
        scroll_view_handler<platform::android>* self = nullptr;
        void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); }
    };
    struct orient_cb_t {
        scroll_view_handler<platform::android>* self = nullptr;
        void operator()(scroll_orientation o) const { self->apply_orientation(o); }
    };

    jobject       native_ = nullptr;  // ScrollView global ref
    basic_scroll_view*  bound_  = nullptr;

    content_cb_t                                  content_cb_{this};
    orient_cb_t                                   orient_cb_{this};
    signal_slot<std::shared_ptr<view> const&>     content_slot_{};
    signal_slot<const scroll_orientation&>        orient_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_SCROLL_VIEW_HANDLER_HPP
