// SPDX-License-Identifier: Apache-2.0
// Android basic_switch_cell handler — horizontal LinearLayout: TextView on
// the left (weight=1), android.widget.Switch on the right. Reuses
// MppCheckedChangeListener with kind=4 (basic_switch_cell) to bounce user
// flips back into the cell's `on` Observable through the shared
// compound-basic_button dispatcher.

#ifndef MPAPP_HANDLERS_ANDROID_SWITCH_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_SWITCH_CELL_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_switch_cell.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class switch_cell_handler<platform::android> {
public:
    switch_cell_handler();
    ~switch_cell_handler();

    switch_cell_handler(const switch_cell_handler&)            = delete;
    switch_cell_handler& operator=(const switch_cell_handler&) = delete;
    switch_cell_handler(switch_cell_handler&&)                 = delete;
    switch_cell_handler& operator=(switch_cell_handler&&)      = delete;

    void map_text(basic_switch_cell& c);
    void map_on(basic_switch_cell& c);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

    // Invoked by compound_button_dispatch when the Switch's
    // OnCheckedChangeListener fires with kind=4.
    void on_native_checked_changed(bool checked);

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_switch_cell& /*x*/) noexcept {}


private:
    void apply_text(const std::string& v);
    void apply_on(bool v);

    struct text_cb_t {
        switch_cell_handler<platform::android>* self;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct on_cb_t {
        switch_cell_handler<platform::android>* self;
        void operator()(bool v) const { self->apply_on(v); }
    };

    jobject       native_      = nullptr;  // global ref LinearLayout
    jobject       text_view_   = nullptr;  // global ref TextView
    jobject       switch_obj_  = nullptr;  // global ref Switch
    jobject       listener_    = nullptr;  // global ref MppCheckedChangeListener
    bool          suppress_echo_ = false;
    basic_switch_cell*  bound_         = nullptr;

    text_cb_t                       text_cb_{this};
    on_cb_t                         on_cb_{this};
    signal_slot<const std::string&> text_slot_{};
    signal_slot<const bool&>        on_slot_{};
};

void android_switch_cell_dispatch_checked_changed(switch_cell_handler<platform::android>* h,
                                                  bool checked);

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_SWITCH_CELL_HANDLER_HPP
