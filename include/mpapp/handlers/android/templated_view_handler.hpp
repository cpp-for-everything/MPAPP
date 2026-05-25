// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android `basic_templated_view` handler — wraps
// `android.widget.FrameLayout`. `template_id` is recorded into a member
// string; rendering still routes through `content` until the templating
// engine ADR lands.

#ifndef MPAPP_HANDLERS_ANDROID_TEMPLATED_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_TEMPLATED_VIEW_HANDLER_HPP

#include <memory>
#include <string>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_templated_view.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class templated_view_handler<platform::android> {
public:
    templated_view_handler();
    ~templated_view_handler();
    templated_view_handler(const templated_view_handler&)            = delete;
    templated_view_handler& operator=(const templated_view_handler&) = delete;
    templated_view_handler(templated_view_handler&&)                 = delete;
    templated_view_handler& operator=(templated_view_handler&&)      = delete;

    void map_content(basic_templated_view& t);
    void map_template_id(basic_templated_view& t);

    void bind_content(basic_templated_view& t, view& child);

    const std::string& template_id() const noexcept { return template_id_; }

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_templated_view& /*x*/) noexcept {}


private:
    void apply_content(const std::shared_ptr<view>& v);
    void apply_template_id(const std::string& v);

    struct content_cb_t     { templated_view_handler<platform::android>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };
    struct template_id_cb_t { templated_view_handler<platform::android>* self; void operator()(const std::string& v)            const { self->apply_template_id(v); } };

    jobject     native_ = nullptr;
    std::string template_id_{};

    content_cb_t                              content_cb_{this};
    template_id_cb_t                          template_id_cb_{this};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
    signal_slot<const std::string&>           template_id_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_TEMPLATED_VIEW_HANDLER_HPP
