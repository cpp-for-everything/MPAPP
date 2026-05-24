// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android `basic_menu_flyout_item` handler — wraps a
// `android.widget.Button`. `text` flows through to `setText`;
// `is_enabled` to `setEnabled`. A native `View.OnClickListener` is
// installed that fires the cross-platform `clicked` signal. Using a
// Button (rather than a TextView) keeps the look closer to a desktop
// menu item without requiring a custom style resource for the M-04b
// surface.

#ifndef MPAPP_HANDLERS_ANDROID_MENU_FLYOUT_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_MENU_FLYOUT_ITEM_HANDLER_HPP

#include <string>

#include "../../internal/basic_menu_flyout_item.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class menu_flyout_item_handler<platform::android> {
public:
    menu_flyout_item_handler();
    ~menu_flyout_item_handler();
    menu_flyout_item_handler(const menu_flyout_item_handler&)            = delete;
    menu_flyout_item_handler& operator=(const menu_flyout_item_handler&) = delete;
    menu_flyout_item_handler(menu_flyout_item_handler&&)                 = delete;
    menu_flyout_item_handler& operator=(menu_flyout_item_handler&&)      = delete;

    void map_text(basic_menu_flyout_item& i);
    void map_is_enabled(basic_menu_flyout_item& i);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_text(const std::string& v);
    void apply_is_enabled(bool v);

    struct text_cb_t       { menu_flyout_item_handler<platform::android>* self; void operator()(const std::string& v) const { self->apply_text(v); } };
    struct is_enabled_cb_t { menu_flyout_item_handler<platform::android>* self; void operator()(bool v)               const { self->apply_is_enabled(v); } };

    jobject           native_   = nullptr;  // android.widget.Button (global ref)
    basic_menu_flyout_item* owner_    = nullptr;
    bool              listener_ = false;

    text_cb_t                          text_cb_{this};
    is_enabled_cb_t                    is_enabled_cb_{this};
    signal_slot<const std::string&>    text_slot_{};
    signal_slot<const bool&>           is_enabled_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_MENU_FLYOUT_ITEM_HANDLER_HPP
