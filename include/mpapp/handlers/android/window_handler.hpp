// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android window handler.
//
// On Android each `mpapp::window` maps to the content view of an
// `android.app.Activity`. The handler stores a global ref to the
// Activity and routes `content` writes through
// `Activity.setContentView(View)`.

#ifndef MPAPP_HANDLERS_ANDROID_WINDOW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_WINDOW_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../window.hpp"

#if defined(__ANDROID__)

#include <jni.h>
#include <string>

namespace mpapp {

template <>
class window_handler<platform::android> {
public:
    window_handler();
    ~window_handler();

    window_handler(const window_handler&)            = delete;
    window_handler& operator=(const window_handler&) = delete;
    window_handler(window_handler&&)                 = delete;
    window_handler& operator=(window_handler&&)      = delete;

    void bind(window& w);

    // Activity jobject (global ref).
    jobject     native() noexcept       { return native_; }
    jobject     native() const noexcept { return native_; }

private:
    void apply_title(const std::string& v);
    void apply_content(view* v);
    void apply_is_visible(bool v);

    struct title_cb_t   { window_handler<platform::android>* self; void operator()(const std::string& v) const { self->apply_title(v); } };
    struct content_cb_t { window_handler<platform::android>* self; void operator()(view* v) const { self->apply_content(v); } };
    struct visible_cb_t { window_handler<platform::android>* self; void operator()(bool v) const { self->apply_is_visible(v); } };

    jobject native_ = nullptr;  // global ref to Activity
    window* bound_  = nullptr;

    title_cb_t                       title_cb_{this};
    content_cb_t                     content_cb_{this};
    visible_cb_t                     visible_cb_{this};
    signal_slot<const std::string&>  title_slot_{};
    signal_slot<view* const&>        content_slot_{};
    signal_slot<const bool&>         visible_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_WINDOW_HANDLER_HPP
