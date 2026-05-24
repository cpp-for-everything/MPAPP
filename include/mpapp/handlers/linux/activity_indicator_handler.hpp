// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_activity_indicator handler — wraps `GtkSpinner`.

#ifndef MPAPP_HANDLERS_LINUX_ACTIVITY_INDICATOR_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_ACTIVITY_INDICATOR_HANDLER_HPP

#include <string>

#include "../../internal/basic_activity_indicator.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class activity_indicator_handler<platform::linux_> {
public:
    activity_indicator_handler();
    ~activity_indicator_handler();

    activity_indicator_handler(const activity_indicator_handler&)            = delete;
    activity_indicator_handler& operator=(const activity_indicator_handler&) = delete;

    void map_is_running(basic_activity_indicator& a);
    void map_color(basic_activity_indicator& a);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_is_running(bool v);
    void apply_color(const brush_ref& b);

    struct is_running_cb_t { activity_indicator_handler<platform::linux_>* self; void operator()(bool v) const { self->apply_is_running(v); } };
    struct color_cb_t      { activity_indicator_handler<platform::linux_>* self; void operator()(const brush_ref& b) const { self->apply_color(b); } };

    void*       native_   = nullptr;   // GtkSpinner*
    void*       provider_ = nullptr;   // GtkCssProvider* (for color)
    std::string class_name_{};

    is_running_cb_t                       is_running_cb_{this};
    color_cb_t                            color_cb_{this};
    signal_slot<const bool&>              is_running_slot_{};
    signal_slot<const brush_ref&>         color_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_ACTIVITY_INDICATOR_HANDLER_HPP
