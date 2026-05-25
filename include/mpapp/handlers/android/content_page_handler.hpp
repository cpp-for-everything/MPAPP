// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_content_page handler — wraps a vertical
// `android.widget.LinearLayout`:
//   - first child: a `TextView` carrying `title`
//   - second child: a `FrameLayout` host for `content`
// Padding is applied via `setPadding` on the outer LinearLayout.

#ifndef MPAPP_HANDLERS_ANDROID_CONTENT_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_CONTENT_PAGE_HANDLER_HPP

#include <memory>
#include <string>

#include "../../internal/basic_content_page.hpp"
#include "../../layout.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class content_page_handler<platform::android> {
public:
    content_page_handler();
    ~content_page_handler();

    content_page_handler(const content_page_handler&)            = delete;
    content_page_handler& operator=(const content_page_handler&) = delete;
    content_page_handler(content_page_handler&&)                 = delete;
    content_page_handler& operator=(content_page_handler&&)      = delete;

    void map_title(basic_content_page& p);
    void map_content(basic_content_page& p);
    void map_padding(basic_content_page& p);

    void bind_content(basic_content_page& p, view& child);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_content_page& /*x*/) noexcept {}


private:
    void apply_title(const std::string& v);
    void apply_content(const std::shared_ptr<view>& v);
    void apply_padding(const thickness& t);

    struct title_cb_t   { content_page_handler<platform::android>* self; void operator()(const std::string& v) const { self->apply_title(v); } };
    struct content_cb_t { content_page_handler<platform::android>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };
    struct padding_cb_t { content_page_handler<platform::android>* self; void operator()(const thickness& t) const { self->apply_padding(t); } };

    jobject native_       = nullptr;  // LinearLayout (vertical) — global ref
    jobject title_view_   = nullptr;  // TextView                — global ref
    jobject content_host_ = nullptr;  // FrameLayout             — global ref

    title_cb_t                                title_cb_{this};
    content_cb_t                              content_cb_{this};
    padding_cb_t                              padding_cb_{this};
    signal_slot<const std::string&>           title_slot_{};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
    signal_slot<const thickness&>             padding_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_CONTENT_PAGE_HANDLER_HPP
