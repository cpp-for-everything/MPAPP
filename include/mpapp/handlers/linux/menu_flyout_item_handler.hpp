// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 `menu_flyout_item` handler — wraps a
// `GtkButton`. `text` flows through to the button's label;
// `is_enabled` toggles `gtk_widget_set_sensitive`. The native
// button's `clicked` signal fires the cross-platform `clicked`
// signal.

#ifndef MPAPP_HANDLERS_LINUX_MENU_FLYOUT_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_MENU_FLYOUT_ITEM_HANDLER_HPP

#include <string>

#include "../../menu_flyout_item.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class menu_flyout_item_handler<platform::linux_> {
public:
    menu_flyout_item_handler();
    ~menu_flyout_item_handler();
    menu_flyout_item_handler(const menu_flyout_item_handler&)            = delete;
    menu_flyout_item_handler& operator=(const menu_flyout_item_handler&) = delete;
    menu_flyout_item_handler(menu_flyout_item_handler&&)                 = delete;
    menu_flyout_item_handler& operator=(menu_flyout_item_handler&&)      = delete;

    void map_text(menu_flyout_item& i);
    void map_is_enabled(menu_flyout_item& i);

    void*       native() noexcept       { return native_; }   // GtkButton*
    const void* native() const noexcept { return native_; }

private:
    void apply_text(const std::string& v);
    void apply_is_enabled(bool v);

    struct text_cb_t       { menu_flyout_item_handler<platform::linux_>* self; void operator()(const std::string& v) const { self->apply_text(v); } };
    struct is_enabled_cb_t { menu_flyout_item_handler<platform::linux_>* self; void operator()(bool v)               const { self->apply_is_enabled(v); } };

    void*             native_   = nullptr;  // GtkButton*
    menu_flyout_item* owner_    = nullptr;
    unsigned long     click_id_ = 0;        // g_signal_handler id

    text_cb_t                          text_cb_{this};
    is_enabled_cb_t                    is_enabled_cb_{this};
    signal_slot<const std::string&>    text_slot_{};
    signal_slot<const bool&>           is_enabled_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_MENU_FLYOUT_ITEM_HANDLER_HPP
