// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android refresh_view handler.
//
// Tries `androidx.swiperefreshlayout.widget.SwipeRefreshLayout` first
// via `FindClass`. If that class is not on the runtime classpath (the
// minimal android_hello example does not link androidx in), falls back
// to a hand-rolled `android.widget.FrameLayout` host with an
// `android.widget.ProgressBar` overlay shown when is_refreshing == true.
//
// Both paths expose the host ViewGroup's jobject (global ref) as the
// native handle. The wrapped content is added as the first child;
// the ProgressBar (fallback path only) is added as the second.

#ifndef MPAPP_HANDLERS_ANDROID_REFRESH_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_REFRESH_VIEW_HANDLER_HPP

#include <memory>

#include "../../platform.hpp"
#include "../../refresh_view.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class refresh_view_handler<platform::android> {
public:
    refresh_view_handler();
    ~refresh_view_handler();

    refresh_view_handler(const refresh_view_handler&)            = delete;
    refresh_view_handler& operator=(const refresh_view_handler&) = delete;
    refresh_view_handler(refresh_view_handler&&)                 = delete;
    refresh_view_handler& operator=(refresh_view_handler&&)      = delete;

    void map_content(refresh_view& r);
    void map_is_refreshing(refresh_view& r);
    void map_refresh_color(refresh_view& r);

    void bind_content(refresh_view& r, view& child);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_content(const std::shared_ptr<view>& v);
    void apply_is_refreshing(bool v);
    void apply_refresh_color(const brush_ref& b);

    struct content_cb_t       { refresh_view_handler<platform::android>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };
    struct is_refreshing_cb_t { refresh_view_handler<platform::android>* self; void operator()(bool v) const { self->apply_is_refreshing(v); } };
    struct refresh_color_cb_t { refresh_view_handler<platform::android>* self; void operator()(const brush_ref& b) const { self->apply_refresh_color(b); } };

    // Host ViewGroup global ref (either SwipeRefreshLayout or FrameLayout).
    jobject native_  = nullptr;
    // Fallback ProgressBar overlay (FrameLayout path only) — null when
    // SwipeRefreshLayout is in use.
    jobject spinner_ = nullptr;
    // True iff `native_` is a SwipeRefreshLayout (so apply_* takes the
    // androidx-path setter calls instead of poking the overlay).
    bool    is_swipe_ = false;

    content_cb_t                              content_cb_{this};
    is_refreshing_cb_t                        is_refreshing_cb_{this};
    refresh_color_cb_t                        refresh_color_cb_{this};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
    signal_slot<const bool&>                  is_refreshing_slot_{};
    signal_slot<const brush_ref&>             refresh_color_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_REFRESH_VIEW_HANDLER_HPP
