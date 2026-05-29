// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android basic_button handler. Wraps
// `android.widget.Button`.

#ifndef MPAPP_HANDLERS_ANDROID_BUTTON_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_BUTTON_HANDLER_HPP

#include "../../internal/basic_button.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>
#include <string>

namespace mpapp::internal {

template <>
class button_handler<platform::android> {
public:
    button_handler();
    ~button_handler();

    button_handler(const button_handler&)            = delete;
    button_handler& operator=(const button_handler&) = delete;
    button_handler(button_handler&&)                 = delete;
    button_handler& operator=(button_handler&&)      = delete;

    void map_text(basic_button& b);
    void map_clicked(basic_button& b);
    void map_semantics(basic_button& b);   // contentDescription (a11y)

    // RFC-0003 stub: Android GestureDetector wire-up pending the
    // Android real-handler task. No-op today.
    void map_gestures(basic_button& /*b*/) noexcept {}

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_text(const std::string& text);
    void apply_semantics(const std::string& desc);

    struct text_callback {
        button_handler<platform::android>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct sem_callback {
        button_handler<platform::android>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_semantics(v); }
    };

    jobject                          native_ = nullptr;  // global ref to Button
    text_callback                    text_cb_{this};
    sem_callback                     sem_cb_{this};
    signal_slot<const std::string&>  text_slot_{};
    signal_slot<const std::string&>  sem_slot_{};
};

// Called by the JNI bridge when the Java Button's onClick fires.
void android_button_dispatch_click(basic_button* b);

} // namespace mpapp::internal

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_BUTTON_HANDLER_HPP
