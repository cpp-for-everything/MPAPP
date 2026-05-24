// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_swipe_item_menu_item handler.
//
// Hosts the action as a `muxc::Button` carrying the `text` content and
// raising the `invoked` signal from the basic_button's Click event. WinUI's
// `muxc::SwipeItem` is the canonical native shape (icon + text)
// but `SwipeItem` is not a `UIElement` — it inherits from
// `DependencyObject` and lives only inside `muxc::SwipeItems`. The
// ADR-0013 dispatch surface returns `UIElement`, so we expose a Button
// here that wins both criteria: it is a UIElement (registers cleanly
// with the dispatch registry) and it carries text + a clickable
// affordance that hosts can also stand alone outside a SwipeControl.
//
// The companion `swipe_view_handler<platform::windows>` separately
// builds a true `muxc::SwipeItems` collection of `muxc::SwipeItem`
// instances when populating SwipeControl.LeftItems / RightItems — see
// that handler's `apply_left_items` / `apply_right_items`.

#ifndef MPAPP_HANDLERS_WINDOWS_SWIPE_ITEM_MENU_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_SWIPE_ITEM_MENU_ITEM_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_swipe_item_menu_item.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.h>

namespace mpapp::internal {

template <>
class swipe_item_menu_item_handler<platform::windows> {
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

    winrt::Microsoft::UI::Xaml::Controls::Button&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Button& native() const noexcept { return native_; }

private:
    void apply_text(const std::string& v);
    void apply_icon_uri(const std::string& v);

    struct text_cb_t {
        swipe_item_menu_item_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct icon_cb_t {
        swipe_item_menu_item_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_icon_uri(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Button native_{nullptr};

    // Owner signal pointer so the Click handler can fire `invoked.emit()`.
    signal<>*                    invoked_signal_ = nullptr;
    winrt::event_token           click_token_{};

    text_cb_t                       text_cb_{this};
    icon_cb_t                       icon_cb_{this};
    signal_slot<const std::string&> text_slot_{};
    signal_slot<const std::string&> icon_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_SWIPE_ITEM_MENU_ITEM_HANDLER_HPP
