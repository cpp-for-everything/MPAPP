// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_progress_bar handler — wraps the horizontal
// `android.widget.ProgressBar` (default style; `setIndeterminate(false)`).

#ifndef MPAPP_HANDLERS_ANDROID_PROGRESS_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_PROGRESS_BAR_HANDLER_HPP

#include "../../platform.hpp"
#include "../../internal/basic_progress_bar.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class progress_bar_handler<platform::android> {
public:
    progress_bar_handler();
    ~progress_bar_handler();
    progress_bar_handler(const progress_bar_handler&)            = delete;
    progress_bar_handler& operator=(const progress_bar_handler&) = delete;

    void map_progress(basic_progress_bar& p);
    void map_color(basic_progress_bar& p);
    void map_background_color(basic_progress_bar& p);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_progress(double v);
    void apply_color(const brush_ref& b);
    void apply_background_color(const brush_ref& b);

    struct progress_cb_t { progress_bar_handler<platform::android>* self; void operator()(double v) const { self->apply_progress(v); } };
    struct color_cb_t    { progress_bar_handler<platform::android>* self; void operator()(const brush_ref& b) const { self->apply_color(b); } };
    struct bg_cb_t       { progress_bar_handler<platform::android>* self; void operator()(const brush_ref& b) const { self->apply_background_color(b); } };

    jobject native_ = nullptr;

    progress_cb_t                 progress_cb_{this};
    color_cb_t                    color_cb_{this};
    bg_cb_t                       bg_cb_{this};
    signal_slot<const double&>    progress_slot_{};
    signal_slot<const brush_ref&> color_slot_{};
    signal_slot<const brush_ref&> bg_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_PROGRESS_BAR_HANDLER_HPP
