// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_menu_bar_item handler — wraps an
// `android.widget.TextView` carrying the basic_entry's `title`. The text view
// is what gets packed into the parent basic_menu_bar's host layout. The
// drop-down menu model is rendered by the parent's `Menu` (see
// `menu_bar_handler.cpp`); the child's `items` collection is observed
// at this level so granular updates can be wired later when the
// basic_menu_flyout family arrives in M-04c.

#ifndef MPAPP_HANDLERS_ANDROID_MENU_BAR_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_MENU_BAR_ITEM_HANDLER_HPP

#include <string>
#include <vector>

#include "../../internal/basic_menu_bar_item.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class menu_bar_item_handler<platform::android> {
public:
    menu_bar_item_handler();
    ~menu_bar_item_handler();
    menu_bar_item_handler(const menu_bar_item_handler&)            = delete;
    menu_bar_item_handler& operator=(const menu_bar_item_handler&) = delete;

    void map_title(basic_menu_bar_item& m);
    void map_items(basic_menu_bar_item& m);

    // Current title text. Read by the parent basic_menu_bar handler when it
    // rebuilds its `Menu` from the child entries — Android has no
    // "MenuBarItem" native widget, so the parent flattens children into
    // MenuItem entries via this accessor.
    const std::string& current_title() const noexcept { return current_title_; }

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_title(const std::string& v);
    void apply_items(const std::vector<view*>& v);

    struct title_cb_t {
        menu_bar_item_handler<platform::android>* self;
        void operator()(const std::string& v) const { self->apply_title(v); }
    };
    struct items_cb_t {
        menu_bar_item_handler<platform::android>* self;
        void operator()(const std::vector<view*>& v) const { self->apply_items(v); }
    };

    jobject     native_        = nullptr;  // TextView (global ref)
    std::string current_title_ = {};

    title_cb_t                              title_cb_{this};
    items_cb_t                              items_cb_{this};
    signal_slot<const std::string&>         title_slot_{};
    signal_slot<std::vector<view*> const&>  items_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_MENU_BAR_ITEM_HANDLER_HPP
