// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android switch handler — wraps android.widget.Switch
// (the Material toggle is `com.google.android.material.switchmaterial.SwitchMaterial`
// but the AOSP Switch widget is sufficient for the spike and avoids the
// Material dependency).

#ifndef MPAPP_HANDLERS_ANDROID_SWITCH_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_SWITCH_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../switch_.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class switch_handler<platform::android> {
public:
    switch_handler();
    ~switch_handler();

    switch_handler(const switch_handler&)            = delete;
    switch_handler& operator=(const switch_handler&) = delete;
    switch_handler(switch_handler&&)                 = delete;
    switch_handler& operator=(switch_handler&&)      = delete;

    void map_is_on(switch_& s);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

    void on_native_checked_changed(bool checked);

private:
    void apply_is_on(bool on);

    struct is_on_callback {
        switch_handler<platform::android>* self = nullptr;
        void operator()(bool v) const { self->apply_is_on(v); }
    };

    jobject                  native_   = nullptr;  // global ref to Switch
    jobject                  listener_ = nullptr;  // global ref to MppCheckedChangeListener
    switch_*                 bound_    = nullptr;
    bool                     suppress_echo_ = false;
    is_on_callback           is_on_cb_{this};
    signal_slot<const bool&> is_on_slot_{};
};

void android_switch_dispatch_checked_changed(switch_handler<platform::android>* h, bool checked);

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_SWITCH_HANDLER_HPP
