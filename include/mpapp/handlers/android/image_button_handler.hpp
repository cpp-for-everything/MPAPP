// SPDX-License-Identifier: Apache-2.0
// Android basic_image_button handler — wraps `android.widget.ImageButton`.

#ifndef MPAPP_HANDLERS_ANDROID_IMAGE_BUTTON_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_IMAGE_BUTTON_HANDLER_HPP

#include "../../internal/basic_image_button.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class image_button_handler<platform::android> {
public:
    image_button_handler();
    ~image_button_handler();
    image_button_handler(const image_button_handler&)            = delete;
    image_button_handler& operator=(const image_button_handler&) = delete;

    void map_source(basic_image_button& b);
    void map_aspect(basic_image_button& b);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_image_button& /*x*/) noexcept {}


private:
    void apply_source(const std::string& v);
    void apply_aspect(aspect_mode v);

    struct source_cb_t { image_button_handler<platform::android>* self; void operator()(const std::string& v) const { self->apply_source(v); } };
    struct aspect_cb_t { image_button_handler<platform::android>* self; void operator()(aspect_mode v) const { self->apply_aspect(v); } };

    jobject native_ = nullptr;

    source_cb_t                        source_cb_{this};
    aspect_cb_t                        aspect_cb_{this};
    signal_slot<const std::string&>    source_slot_{};
    signal_slot<const aspect_mode&>    aspect_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_IMAGE_BUTTON_HANDLER_HPP
