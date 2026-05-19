// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android slider handler — wraps android.widget.SeekBar.
//
// SeekBar uses integer progress; the handler maps the cross-platform
// double `value` into [0, kSeekResolution] for native storage. This
// gives us ~10000 distinct positions which is well past human visual
// resolution; if higher precision is ever needed the resolution
// constant lifts to 1e6 with no API change.

#ifndef MPAPP_HANDLERS_ANDROID_SLIDER_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_SLIDER_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../slider.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class slider_handler<platform::android> {
public:
    static constexpr int kSeekResolution = 10000;

    slider_handler();
    ~slider_handler();

    slider_handler(const slider_handler&)            = delete;
    slider_handler& operator=(const slider_handler&) = delete;
    slider_handler(slider_handler&&)                 = delete;
    slider_handler& operator=(slider_handler&&)      = delete;

    void map_value(slider& s);
    void map_minimum(slider& s);
    void map_maximum(slider& s);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

    // Called by the JNI trampoline when the SeekBar progress changes.
    void on_native_progress_changed(int progress, int max);

private:
    void apply_value(double v);
    void apply_minimum(double v);
    void apply_maximum(double v);

    void update_native_progress_from_value();

    struct value_cb_t   { slider_handler<platform::android>* self = nullptr; void operator()(double v) const { self->apply_value(v); } };
    struct minimum_cb_t { slider_handler<platform::android>* self = nullptr; void operator()(double v) const { self->apply_minimum(v); } };
    struct maximum_cb_t { slider_handler<platform::android>* self = nullptr; void operator()(double v) const { self->apply_maximum(v); } };

    jobject     native_   = nullptr;  // global ref to SeekBar
    jobject     listener_ = nullptr;
    slider*     bound_    = nullptr;
    bool        suppress_echo_ = false;

    value_cb_t                  value_cb_{this};
    minimum_cb_t                minimum_cb_{this};
    maximum_cb_t                maximum_cb_{this};
    signal_slot<const double&>  value_slot_{};
    signal_slot<const double&>  minimum_slot_{};
    signal_slot<const double&>  maximum_slot_{};
};

void android_slider_dispatch_progress(slider_handler<platform::android>* h, int progress, int max);

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_SLIDER_HANDLER_HPP
