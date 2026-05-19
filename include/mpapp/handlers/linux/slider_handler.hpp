// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 slider handler — wraps GtkScale.

#ifndef MPAPP_HANDLERS_LINUX_SLIDER_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_SLIDER_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../slider.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class slider_handler<platform::linux_> {
public:
    slider_handler();
    ~slider_handler();

    slider_handler(const slider_handler&)            = delete;
    slider_handler& operator=(const slider_handler&) = delete;
    slider_handler(slider_handler&&)                 = delete;
    slider_handler& operator=(slider_handler&&)      = delete;

    void map_value(slider& s);
    void map_minimum(slider& s);
    void map_maximum(slider& s);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

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

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_SLIDER_HANDLER_HPP
