// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_page handler — wraps a vertical
// `android.widget.LinearLayout`:
//   - first child: a `TextView` carrying `title`
//   - second child: a `FrameLayout` host for `content`
//   - overlay: an indeterminate `ProgressBar` toggled by `is_busy`
//     (added/removed dynamically so the layout stays clean)
// This mirrors the structure of `content_page_handler<platform::android>`.

#ifndef MPAPP_HANDLERS_ANDROID_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_PAGE_HANDLER_HPP

#include <string>

#include "../../internal/basic_page.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class page_handler<platform::android> {
public:
    page_handler();
    ~page_handler();

    page_handler(const page_handler&)            = delete;
    page_handler& operator=(const page_handler&) = delete;
    page_handler(page_handler&&)                 = delete;
    page_handler& operator=(page_handler&&)      = delete;

    void map_title(basic_page& p);
    void map_content(basic_page& p);
    void map_is_busy(basic_page& p);

    void bind_content(basic_page& p, view& child);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_page& /*x*/) noexcept {}


private:
    void apply_title(const std::string& v);
    void apply_content(view* v);
    void apply_is_busy(bool v);

    struct title_cb_t   { page_handler<platform::android>* self; void operator()(const std::string& v) const { self->apply_title(v); } };
    struct content_cb_t { page_handler<platform::android>* self; void operator()(view* v) const { self->apply_content(v); } };
    struct busy_cb_t    { page_handler<platform::android>* self; void operator()(bool v) const { self->apply_is_busy(v); } };

    jobject native_       = nullptr;  // LinearLayout (vertical) — global ref
    jobject title_view_   = nullptr;  // TextView                — global ref
    jobject content_host_ = nullptr;  // FrameLayout             — global ref
    jobject busy_bar_     = nullptr;  // ProgressBar             — global ref

    title_cb_t                       title_cb_{this};
    content_cb_t                     content_cb_{this};
    busy_cb_t                        busy_cb_{this};
    signal_slot<const std::string&>  title_slot_{};
    signal_slot<view* const&>        content_slot_{};
    signal_slot<const bool&>         busy_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_PAGE_HANDLER_HPP
