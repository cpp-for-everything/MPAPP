// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android `basic_menu_flyout_sub_item` handler — wraps a
// vertical `LinearLayout` containing a `TextView` basic_label (the sub-menu
// header) followed by an inner vertical `LinearLayout` host for the
// submenu items. `text` flows to the basic_label's `setText`; `items`
// rebuilds the inner host from the ADR-0013 dispatch registry.

#ifndef MPAPP_HANDLERS_ANDROID_MENU_FLYOUT_SUB_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_MENU_FLYOUT_SUB_ITEM_HANDLER_HPP

#include <string>
#include <vector>

#include "../../internal/basic_menu_flyout_sub_item.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class menu_flyout_sub_item_handler<platform::android> {
public:
    menu_flyout_sub_item_handler();
    ~menu_flyout_sub_item_handler();
    menu_flyout_sub_item_handler(const menu_flyout_sub_item_handler&)            = delete;
    menu_flyout_sub_item_handler& operator=(const menu_flyout_sub_item_handler&) = delete;
    menu_flyout_sub_item_handler(menu_flyout_sub_item_handler&&)                 = delete;
    menu_flyout_sub_item_handler& operator=(menu_flyout_sub_item_handler&&)      = delete;

    void map_text(basic_menu_flyout_sub_item& s);
    void map_items(basic_menu_flyout_sub_item& s);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_menu_flyout_sub_item& /*x*/) noexcept {}


private:
    void apply_text(const std::string& v);
    void apply_items(const std::vector<view*>& v);

    struct text_cb_t  { menu_flyout_sub_item_handler<platform::android>* self; void operator()(const std::string& v)        const { self->apply_text(v); } };
    struct items_cb_t { menu_flyout_sub_item_handler<platform::android>* self; void operator()(const std::vector<view*>& v) const { self->apply_items(v); } };

    jobject native_     = nullptr;  // outer LinearLayout (vertical, global ref)
    jobject label_      = nullptr;  // TextView                 (global ref)
    jobject child_host_ = nullptr;  // inner LinearLayout       (global ref)

    text_cb_t                                  text_cb_{this};
    items_cb_t                                 items_cb_{this};
    signal_slot<const std::string&>            text_slot_{};
    signal_slot<std::vector<view*> const&>     items_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_MENU_FLYOUT_SUB_ITEM_HANDLER_HPP
