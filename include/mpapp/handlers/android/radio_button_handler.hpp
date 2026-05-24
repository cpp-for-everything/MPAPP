// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_radio_button handler.
//
// Wraps android.widget.RadioButton. Native grouping is via
// android.widget.RadioGroup; the handler maintains a per-app
// group_name → RadioGroup global-ref registry so multiple
// radio_buttons sharing the same group_name attach to the same
// RadioGroup.

#ifndef MPAPP_HANDLERS_ANDROID_RADIO_BUTTON_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_RADIO_BUTTON_HANDLER_HPP

#include "../../platform.hpp"
#include "../../internal/basic_radio_button.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>
#include <string>

namespace mpapp::internal {

template <>
class radio_button_handler<platform::android> {
public:
    radio_button_handler();
    ~radio_button_handler();

    radio_button_handler(const radio_button_handler&)            = delete;
    radio_button_handler& operator=(const radio_button_handler&) = delete;
    radio_button_handler(radio_button_handler&&)                 = delete;
    radio_button_handler& operator=(radio_button_handler&&)      = delete;

    void map_is_checked(basic_radio_button& r);
    void map_group_name(basic_radio_button& r);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

    static constexpr int kind = 3;
    void on_native_checked_changed(bool checked);

private:
    void apply_is_checked(bool v);
    void apply_group_name(const std::string& v);

    struct is_checked_cb_t {
        radio_button_handler<platform::android>* self = nullptr;
        void operator()(bool v) const { self->apply_is_checked(v); }
    };
    struct group_name_cb_t {
        radio_button_handler<platform::android>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_group_name(v); }
    };

    jobject                  native_      = nullptr;  // RadioButton
    jobject                  listener_    = nullptr;
    basic_radio_button*            bound_       = nullptr;
    std::string              attached_group_{};
    bool                     suppress_echo_ = false;

    is_checked_cb_t                  is_checked_cb_{this};
    group_name_cb_t                  group_name_cb_{this};
    signal_slot<const bool&>         is_checked_slot_{};
    signal_slot<const std::string&>  group_name_slot_{};
};

void android_radio_button_dispatch_checked_changed(radio_button_handler<platform::android>* h, bool checked);

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_RADIO_BUTTON_HANDLER_HPP
