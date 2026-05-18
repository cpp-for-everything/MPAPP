// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android label handler. Wraps
// `android.widget.TextView`.

#ifndef MPAPP_HANDLERS_ANDROID_LABEL_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_LABEL_HANDLER_HPP

#include "../../label.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>
#include <string>

namespace mpapp {

template <>
class label_handler<platform::android> {
public:
    label_handler();
    ~label_handler();

    label_handler(const label_handler&)            = delete;
    label_handler& operator=(const label_handler&) = delete;
    label_handler(label_handler&&)                 = delete;
    label_handler& operator=(label_handler&&)      = delete;

    void map_text(label& l);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_text(const std::string& text);

    struct text_callback {
        label_handler<platform::android>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };

    jobject                          native_ = nullptr;  // global ref to TextView
    text_callback                    text_cb_{this};
    signal_slot<const std::string&>  text_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_LABEL_HANDLER_HPP
