// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_flyout_view handler.
//
// Tries `androidx.drawerlayout.widget.DrawerLayout` first via
// `FindClass`. If that class is not on the runtime classpath (the
// minimal `android_hello` example does not link androidx in), falls
// back to a hand-rolled `android.widget.LinearLayout` (horizontal)
// host with the flyout pane shown/hidden by toggling its View
// visibility — preserving the cross-platform contract of the widget.
//
// Both paths expose the host ViewGroup's jobject (global ref) as the
// native handle. The drawer pane is added with Gravity.START on the
// DrawerLayout path; on the LinearLayout path it is added as the
// first child and the detail pane is added second.

#ifndef MPAPP_HANDLERS_ANDROID_FLYOUT_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_FLYOUT_VIEW_HANDLER_HPP

#include <memory>

#include "../../internal/basic_flyout_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class flyout_view_handler<platform::android> {
public:
    flyout_view_handler();
    ~flyout_view_handler();

    flyout_view_handler(const flyout_view_handler&)            = delete;
    flyout_view_handler& operator=(const flyout_view_handler&) = delete;
    flyout_view_handler(flyout_view_handler&&)                 = delete;
    flyout_view_handler& operator=(flyout_view_handler&&)      = delete;

    void map_flyout(basic_flyout_view& f);
    void map_detail(basic_flyout_view& f);
    void map_is_presented(basic_flyout_view& f);

    void bind_flyout(basic_flyout_view& f, view& child);
    void bind_detail(basic_flyout_view& f, view& child);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_flyout(const std::shared_ptr<view>& v);
    void apply_detail(const std::shared_ptr<view>& v);
    void apply_is_presented(bool v);

    struct flyout_cb_t       { flyout_view_handler<platform::android>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_flyout(v); } };
    struct detail_cb_t       { flyout_view_handler<platform::android>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_detail(v); } };
    struct is_presented_cb_t { flyout_view_handler<platform::android>* self; void operator()(bool v) const { self->apply_is_presented(v); } };

    // Host ViewGroup global ref (either DrawerLayout or LinearLayout).
    jobject native_         = nullptr;
    // Current attached flyout pane child (global ref, owned by us in
    // addition to the parent's own ref-keeping — released in dtor + on
    // replacement). Tracked so we can openDrawer/closeDrawer or set
    // visibility on it directly.
    jobject current_flyout_ = nullptr;
    // Current attached detail pane child (global ref, same lifecycle).
    jobject current_detail_ = nullptr;
    // True iff `native_` is a DrawerLayout (so apply_is_presented takes
    // the androidx-path open/close calls instead of toggling visibility).
    bool    is_drawer_      = false;

    flyout_cb_t                               flyout_cb_{this};
    detail_cb_t                               detail_cb_{this};
    is_presented_cb_t                         is_presented_cb_{this};
    signal_slot<std::shared_ptr<view> const&> flyout_slot_{};
    signal_slot<std::shared_ptr<view> const&> detail_slot_{};
    signal_slot<const bool&>                  is_presented_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_FLYOUT_VIEW_HANDLER_HPP
