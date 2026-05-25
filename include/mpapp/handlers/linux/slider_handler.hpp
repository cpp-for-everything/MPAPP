// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_slider handler — wraps GtkScale.

#ifndef MPAPP_HANDLERS_LINUX_SLIDER_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_SLIDER_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_slider.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class slider_handler<platform::linux_> {
public:
    slider_handler();
    ~slider_handler();

    slider_handler(const slider_handler&)            = delete;
    slider_handler& operator=(const slider_handler&) = delete;
    slider_handler(slider_handler&&)                 = delete;
    slider_handler& operator=(slider_handler&&)      = delete;

    void map_value(basic_slider& s);
    void map_minimum(basic_slider& s);
    void map_maximum(basic_slider& s);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_slider& x);


private:
    void apply_value(double v);
    void apply_minimum(double v);
    void apply_maximum(double v);

    struct value_cb_t   { slider_handler<platform::linux_>* self = nullptr; void operator()(double v) const { self->apply_value(v); } };
    struct minimum_cb_t { slider_handler<platform::linux_>* self = nullptr; void operator()(double v) const { self->apply_minimum(v); } };
    struct maximum_cb_t { slider_handler<platform::linux_>* self = nullptr; void operator()(double v) const { self->apply_maximum(v); } };

    void*                       native_              = nullptr;  // GtkScale*
    unsigned long               value_changed_handler_id_ = 0;
    bool                        suppress_echo_       = false;

    value_cb_t                  value_cb_{this};
    minimum_cb_t                minimum_cb_{this};
    maximum_cb_t                maximum_cb_{this};
    signal_slot<const double&>  value_slot_{};
    signal_slot<const double&>  minimum_slot_{};
    signal_slot<const double&>  maximum_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_SLIDER_HANDLER_HPP
