// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android `basic_menu_flyout_separator` handler — wraps a
// thin `android.view.View` styled as a horizontal divider. Android's
// `Menu` API doesn't render dividers between menu items by default,
// so the handler injects a 1-px-tall plain view; the parent basic_menu_flyout
// packs it between sibling items.

#ifndef MPAPP_HANDLERS_ANDROID_MENU_FLYOUT_SEPARATOR_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_MENU_FLYOUT_SEPARATOR_HANDLER_HPP

#include "../../internal/basic_menu_flyout_separator.hpp"
#include "../../platform.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class menu_flyout_separator_handler<platform::android> {
public:
    menu_flyout_separator_handler();
    ~menu_flyout_separator_handler();
    menu_flyout_separator_handler(const menu_flyout_separator_handler&)            = delete;
    menu_flyout_separator_handler& operator=(const menu_flyout_separator_handler&) = delete;
    menu_flyout_separator_handler(menu_flyout_separator_handler&&)                 = delete;
    menu_flyout_separator_handler& operator=(menu_flyout_separator_handler&&)      = delete;

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_menu_flyout_separator& /*x*/) noexcept {}


private:
    jobject native_ = nullptr;  // android.view.View (global ref)
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_MENU_FLYOUT_SEPARATOR_HANDLER_HPP
