// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_box_view handler — colored rectangle with
// rounded corners. Implemented as `android.view.View` with a
// `GradientDrawable` background carrying the fill color and per-corner
// radii.

#ifndef MPAPP_HANDLERS_ANDROID_BOX_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_BOX_VIEW_HANDLER_HPP

#include "../../internal/basic_box_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class box_view_handler<platform::android> {
public:
    box_view_handler();
    ~box_view_handler();

    box_view_handler(const box_view_handler&)            = delete;
    box_view_handler& operator=(const box_view_handler&) = delete;
    box_view_handler(box_view_handler&&)                 = delete;
    box_view_handler& operator=(box_view_handler&&)      = delete;

    void map_fill(basic_box_view& b);
    void map_corners(basic_box_view& b);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_fill(const color& c);
    void apply_corners(const corner_radius& r);
    void rebuild_background();

    struct fill_cb_t {
        box_view_handler<platform::android>* self = nullptr;
        void operator()(const color& c) const { self->apply_fill(c); }
    };
    struct corners_cb_t {
        box_view_handler<platform::android>* self = nullptr;
        void operator()(const corner_radius& r) const { self->apply_corners(r); }
    };

    jobject native_ = nullptr;  // View global ref

    color         cached_fill_{};
    corner_radius cached_corners_{};

    fill_cb_t                            fill_cb_{this};
    corners_cb_t                         corners_cb_{this};
    signal_slot<const color&>            fill_slot_{};
    signal_slot<const corner_radius&>    corners_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_BOX_VIEW_HANDLER_HPP
