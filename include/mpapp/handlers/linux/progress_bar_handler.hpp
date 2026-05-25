// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_progress_bar handler — wraps `GtkProgressBar`.

#ifndef MPAPP_HANDLERS_LINUX_PROGRESS_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_PROGRESS_BAR_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../internal/basic_progress_bar.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class progress_bar_handler<platform::linux_> {
public:
    progress_bar_handler();
    ~progress_bar_handler();
    progress_bar_handler(const progress_bar_handler&)            = delete;
    progress_bar_handler& operator=(const progress_bar_handler&) = delete;

    void map_progress(basic_progress_bar& p);
    void map_color(basic_progress_bar& p);
    void map_background_color(basic_progress_bar& p);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_progress_bar& x);


private:
    void apply_progress(double v);
    void apply_color(const brush_ref& b);
    void apply_background_color(const brush_ref& b);
    void reload_css();

    struct progress_cb_t { progress_bar_handler<platform::linux_>* self; void operator()(double v) const { self->apply_progress(v); } };
    struct color_cb_t    { progress_bar_handler<platform::linux_>* self; void operator()(const brush_ref& b) const { self->apply_color(b); } };
    struct bg_cb_t       { progress_bar_handler<platform::linux_>* self; void operator()(const brush_ref& b) const { self->apply_background_color(b); } };

    void*       native_   = nullptr;  // GtkProgressBar*
    void*       provider_ = nullptr;  // GtkCssProvider*
    std::string class_name_{};

    brush_ref cached_color_{};
    brush_ref cached_bg_{};

    progress_cb_t                 progress_cb_{this};
    color_cb_t                    color_cb_{this};
    bg_cb_t                       bg_cb_{this};
    signal_slot<const double&>    progress_slot_{};
    signal_slot<const brush_ref&> color_slot_{};
    signal_slot<const brush_ref&> bg_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_PROGRESS_BAR_HANDLER_HPP
