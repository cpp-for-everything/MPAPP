// SPDX-License-Identifier: Apache-2.0
// Android basic_content_view handler — wraps `android.widget.FrameLayout`.

#ifndef MPAPP_HANDLERS_ANDROID_CONTENT_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_CONTENT_VIEW_HANDLER_HPP

#include <memory>

#include "../../internal/basic_content_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class content_view_handler<platform::android> {
public:
    content_view_handler();
    ~content_view_handler();
    content_view_handler(const content_view_handler&)            = delete;
    content_view_handler& operator=(const content_view_handler&) = delete;

    void map_content(basic_content_view& c);
    void bind_content(basic_content_view& c, view& child);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_content(const std::shared_ptr<view>& v);

    struct content_cb_t { content_view_handler<platform::android>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };

    jobject native_ = nullptr;

    content_cb_t                              content_cb_{this};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_CONTENT_VIEW_HANDLER_HPP
