// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_scroll_view handler — wraps GtkScrolledWindow.

#ifndef MPAPP_HANDLERS_LINUX_SCROLL_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_SCROLL_VIEW_HANDLER_HPP

#include <memory>

#include "../../platform.hpp"
#include "../../internal/basic_scroll_view.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class scroll_view_handler<platform::linux_> {
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

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_scroll_view& x);


private:
    void apply_content(const std::shared_ptr<view>& v);
    void apply_orientation(scroll_orientation o);

    struct content_cb_t {
        scroll_view_handler<platform::linux_>* self = nullptr;
        void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); }
    };
    struct orient_cb_t {
        scroll_view_handler<platform::linux_>* self = nullptr;
        void operator()(scroll_orientation o) const { self->apply_orientation(o); }
    };

    void* native_ = nullptr;  // GtkScrolledWindow*
    basic_scroll_view* bound_ = nullptr;

    content_cb_t                                  content_cb_{this};
    orient_cb_t                                   orient_cb_{this};
    signal_slot<std::shared_ptr<view> const&>     content_slot_{};
    signal_slot<const scroll_orientation&>        orient_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_SCROLL_VIEW_HANDLER_HPP
