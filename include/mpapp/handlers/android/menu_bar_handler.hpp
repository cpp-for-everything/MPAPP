// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android menu_bar handler — wraps
// `androidx.appcompat.widget.Toolbar` (falling back to
// `android.widget.Toolbar`) configured as a menu host. Each child
// `menu_bar_item`'s `title` is added as a `MenuItem` entry on the
// toolbar's `Menu` (Toolbar.getMenu().add(...) / .clear()). On Android
// there is no top-level menu bar surface — the entries collapse into
// the action-bar overflow on tap, which mirrors MAUI's behaviour on
// the platform.

#ifndef MPAPP_HANDLERS_ANDROID_MENU_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_MENU_BAR_HANDLER_HPP

#include <vector>

#include "../../menu_bar.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class menu_bar_handler<platform::android> {
public:
    menu_bar_handler();
    ~menu_bar_handler();
    menu_bar_handler(const menu_bar_handler&)            = delete;
    menu_bar_handler& operator=(const menu_bar_handler&) = delete;

    void map_items(menu_bar& b);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_items(const std::vector<view*>& v);

    struct items_cb_t {
        menu_bar_handler<platform::android>* self;
        void operator()(const std::vector<view*>& v) const { self->apply_items(v); }
    };

    jobject native_ = nullptr;  // android.widget.Toolbar (global ref)

    items_cb_t                              items_cb_{this};
    signal_slot<std::vector<view*> const&>  items_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_MENU_BAR_HANDLER_HPP
