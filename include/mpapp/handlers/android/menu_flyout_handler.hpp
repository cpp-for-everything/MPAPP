// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android `basic_menu_flyout` handler — wraps a vertical
// `LinearLayout` as the items host. `is_open` toggles the layout's
// visibility (VISIBLE / GONE). The "anchored popup" half (PopupMenu
// or PopupWindow) lands with the M-05 context-flyout wiring; until
// then the layout renders inline when shown — same fallback pattern
// basic_flyout_view's Android handler uses when DrawerLayout is missing.

#ifndef MPAPP_HANDLERS_ANDROID_MENU_FLYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_MENU_FLYOUT_HANDLER_HPP

#include <vector>

#include "../../internal/basic_menu_flyout.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class menu_flyout_handler<platform::android> {
public:
    menu_flyout_handler();
    ~menu_flyout_handler();
    menu_flyout_handler(const menu_flyout_handler&)            = delete;
    menu_flyout_handler& operator=(const menu_flyout_handler&) = delete;
    menu_flyout_handler(menu_flyout_handler&&)                 = delete;
    menu_flyout_handler& operator=(menu_flyout_handler&&)      = delete;

    void map_items(basic_menu_flyout& f);
    void map_is_open(basic_menu_flyout& f);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_items(const std::vector<view*>& v);
    void apply_is_open(bool v);

    struct items_cb_t   { menu_flyout_handler<platform::android>* self; void operator()(const std::vector<view*>& v) const { self->apply_items(v); } };
    struct is_open_cb_t { menu_flyout_handler<platform::android>* self; void operator()(bool v)                      const { self->apply_is_open(v); } };

    jobject native_ = nullptr;  // android.widget.LinearLayout (vertical, global ref)

    items_cb_t                                items_cb_{this};
    is_open_cb_t                              is_open_cb_{this};
    signal_slot<std::vector<view*> const&>    items_slot_{};
    signal_slot<const bool&>                  is_open_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_MENU_FLYOUT_HANDLER_HPP
