// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_check_box handler — wraps android.widget.CheckBox.

#ifndef MPAPP_HANDLERS_ANDROID_CHECK_BOX_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_CHECK_BOX_HANDLER_HPP

#include "../../internal/basic_check_box.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class check_box_handler<platform::android> {
public:
    check_box_handler();
    ~check_box_handler();

    check_box_handler(const check_box_handler&)            = delete;
    check_box_handler& operator=(const check_box_handler&) = delete;
    check_box_handler(check_box_handler&&)                 = delete;
    check_box_handler& operator=(check_box_handler&&)      = delete;

    void map_is_checked(basic_check_box& c);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

    // Discriminator so the shared MppCheckedChangeListener trampoline
    // can route into either switch_handler or check_box_handler. See
    // android_compound_dispatch_checked_changed below.
    static constexpr int kind = 2;
    void on_native_checked_changed(bool checked);

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_check_box& /*x*/) noexcept {}


private:
    void apply_is_checked(bool v);

    struct cb_t {
        check_box_handler<platform::android>* self = nullptr;
        void operator()(bool v) const { self->apply_is_checked(v); }
    };

    jobject                  native_   = nullptr;
    jobject                  listener_ = nullptr;
    basic_check_box*               bound_    = nullptr;
    bool                     suppress_echo_ = false;
    cb_t                     cb_{this};
    signal_slot<const bool&> slot_{};
};

void android_check_box_dispatch_checked_changed(check_box_handler<platform::android>* h, bool checked);

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_CHECK_BOX_HANDLER_HPP
