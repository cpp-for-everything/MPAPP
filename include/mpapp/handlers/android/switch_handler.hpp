// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android switch handler — wraps android.widget.Switch
// (the Material toggle is `com.google.android.material.switchmaterial.SwitchMaterial`
// but the AOSP Switch widget is sufficient for the spike and avoids the
// Material dependency).

#ifndef MPAPP_HANDLERS_ANDROID_SWITCH_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_SWITCH_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_switch_.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class switch_handler<platform::android> {
public:
    switch_handler();
    ~switch_handler();

    switch_handler(const switch_handler&)            = delete;
    switch_handler& operator=(const switch_handler&) = delete;
    switch_handler(switch_handler&&)                 = delete;
    switch_handler& operator=(switch_handler&&)      = delete;

    void map_is_on(basic_switch_& s);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

    void on_native_checked_changed(bool checked);

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_switch_& /*x*/) noexcept {}


private:
    void apply_is_on(bool on);

    struct is_on_callback {
        switch_handler<platform::android>* self = nullptr;
        void operator()(bool v) const { self->apply_is_on(v); }
    };

    jobject                  native_   = nullptr;  // global ref to Switch
    jobject                  listener_ = nullptr;  // global ref to MppCheckedChangeListener
    basic_switch_*                 bound_    = nullptr;
    bool                     suppress_echo_ = false;
    is_on_callback           is_on_cb_{this};
    signal_slot<const bool&> is_on_slot_{};
};

void android_switch_dispatch_checked_changed(switch_handler<platform::android>* h, bool checked);

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_SWITCH_HANDLER_HPP
