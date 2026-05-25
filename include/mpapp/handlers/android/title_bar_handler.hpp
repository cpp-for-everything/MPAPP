// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_title_bar handler — wraps
// `android.widget.Toolbar` (platform jar, API 21+) configured as a
// titlebar. Surfaces the basic_toolbar's title + subtitle text slots.

#ifndef MPAPP_HANDLERS_ANDROID_TITLE_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_TITLE_BAR_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_title_bar.hpp"

#if defined(__ANDROID__)

#include <jni.h>
#include <string>

namespace mpapp::internal {

template <>
class title_bar_handler<platform::android> {
public:
    title_bar_handler();
    ~title_bar_handler();

    title_bar_handler(const title_bar_handler&)            = delete;
    title_bar_handler& operator=(const title_bar_handler&) = delete;
    title_bar_handler(title_bar_handler&&)                 = delete;
    title_bar_handler& operator=(title_bar_handler&&)      = delete;

    void map_title(basic_title_bar& t);
    void map_subtitle(basic_title_bar& t);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_title_bar& /*x*/) noexcept {}


private:
    void apply_title(const std::string& v);
    void apply_subtitle(const std::string& v);

    struct title_cb_t    { title_bar_handler<platform::android>* self; void operator()(const std::string& v) const { self->apply_title(v); } };
    struct subtitle_cb_t { title_bar_handler<platform::android>* self; void operator()(const std::string& v) const { self->apply_subtitle(v); } };

    jobject  native_ = nullptr;  // global ref to android.widget.Toolbar

    title_cb_t                       title_cb_{this};
    subtitle_cb_t                    subtitle_cb_{this};
    signal_slot<const std::string&>  title_slot_{};
    signal_slot<const std::string&>  subtitle_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_TITLE_BAR_HANDLER_HPP
