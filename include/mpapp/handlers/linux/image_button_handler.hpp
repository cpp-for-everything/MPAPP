// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_image_button handler — GtkButton with a GtkPicture child.

#ifndef MPAPP_HANDLERS_LINUX_IMAGE_BUTTON_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_IMAGE_BUTTON_HANDLER_HPP

#include "../../internal/basic_image_button.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class image_button_handler<platform::linux_> {
public:
    image_button_handler();
    ~image_button_handler();
    image_button_handler(const image_button_handler&)            = delete;
    image_button_handler& operator=(const image_button_handler&) = delete;

    void map_source(basic_image_button& b);
    void map_aspect(basic_image_button& b);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_image_button& x);


private:
    void apply_source(const std::string& v);
    void apply_aspect(aspect_mode v);

    struct source_cb_t { image_button_handler<platform::linux_>* self; void operator()(const std::string& v) const { self->apply_source(v); } };
    struct aspect_cb_t { image_button_handler<platform::linux_>* self; void operator()(aspect_mode v) const { self->apply_aspect(v); } };

    void* native_  = nullptr;   // GtkButton*
    void* picture_ = nullptr;   // GtkPicture*

    source_cb_t                        source_cb_{this};
    aspect_cb_t                        aspect_cb_{this};
    signal_slot<const std::string&>    source_slot_{};
    signal_slot<const aspect_mode&>    aspect_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_IMAGE_BUTTON_HANDLER_HPP
