// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_swipe_item_menu_item handler.
//
// `GtkButton` styled as a pill (basic_label + optional icon name). Click on
// the basic_button fires the cross-platform `invoked` signal. The host
// `basic_swipe_view`'s reveal gesture is deferred to a follow-up batch, but
// the action basic_button is still functional as a standalone activator.

#ifndef MPAPP_HANDLERS_LINUX_SWIPE_ITEM_MENU_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_SWIPE_ITEM_MENU_ITEM_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_swipe_item_menu_item.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class swipe_item_menu_item_handler<platform::linux_> {
public:
    swipe_item_menu_item_handler();
    ~swipe_item_menu_item_handler();

    swipe_item_menu_item_handler(const swipe_item_menu_item_handler&)            = delete;
    swipe_item_menu_item_handler& operator=(const swipe_item_menu_item_handler&) = delete;
    swipe_item_menu_item_handler(swipe_item_menu_item_handler&&)                 = delete;
    swipe_item_menu_item_handler& operator=(swipe_item_menu_item_handler&&)      = delete;

    void map_text(basic_swipe_item_menu_item& m);
    void map_icon_uri(basic_swipe_item_menu_item& m);
    void map_invoked(basic_swipe_item_menu_item& m);

    // GtkButton (GtkWidget*) type-erased.
    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_text(const std::string& v);
    void apply_icon_uri(const std::string& v);

    struct text_cb_t {
        swipe_item_menu_item_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct icon_cb_t {
        swipe_item_menu_item_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_icon_uri(v); }
    };

    void* native_ = nullptr; // GtkButton (GtkWidget*)

    // The `clicked` GTK signal trampoline is the file-local
    // `on_swipe_item_clicked` in the .cpp. It receives a
    // `signal<>*` cookie pointing at `invoked_signal_` below.
    signal<>* invoked_signal_ = nullptr;
    unsigned long clicked_handler_id_ = 0;  // g_signal_handler_id_t (gulong)

    text_cb_t                       text_cb_{this};
    icon_cb_t                       icon_cb_{this};
    signal_slot<const std::string&> text_slot_{};
    signal_slot<const std::string&> icon_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_SWIPE_ITEM_MENU_ITEM_HANDLER_HPP
