// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_carousel_view handler.
//
// Native widget: mux::Controls::FlipView — the exact WinUI analog of
// MAUI's CarouselView (paged, swipeable item host). FlipView.SelectedIndex
// is the two-way Position; SelectionChanged drives position +
// position_changed back into the surface. items_source maps to FlipView
// .Items (boxed strings). loop / is_swipe_enabled / peek_count have no
// direct FlipView surface in WinUI 3 — loop/clamp is handled in
// basic_carousel_view::scroll_to; swipe is always touch-enabled; peek is a
// v1 no-op. native_ (the FlipView) is the stable ADR-0013 dispatch handle.

#ifndef MPAPP_HANDLERS_WINDOWS_CAROUSEL_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_CAROUSEL_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../internal/basic_carousel_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class carousel_view_handler<platform::windows> {
public:
    carousel_view_handler();
    ~carousel_view_handler();

    carousel_view_handler(const carousel_view_handler&)            = delete;
    carousel_view_handler& operator=(const carousel_view_handler&) = delete;
    carousel_view_handler(carousel_view_handler&&)                 = delete;
    carousel_view_handler& operator=(carousel_view_handler&&)      = delete;

    void map_items_source(basic_carousel_view& c);
    void map_position(basic_carousel_view& c);
    void map_loop(basic_carousel_view& c);
    void map_is_swipe_enabled(basic_carousel_view& c);
    void map_peek_count(basic_carousel_view& c);
    void map_gestures(basic_carousel_view& c);

    winrt::Microsoft::UI::Xaml::Controls::FlipView&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::FlipView& native() const noexcept { return native_; }

private:
    void rebuild_items(const std::vector<std::string>& v);
    void apply_position(int idx);
    void wire_selection_changed();

    struct items_cb_t {
        carousel_view_handler<platform::windows>* self;
        void operator()(const std::vector<std::string>&) const {
            self->rebuild_items(self->bound_ != nullptr
                                    ? self->bound_->items_source.get()
                                    : std::vector<std::string>{});
        }
    };
    struct pos_cb_t {
        carousel_view_handler<platform::windows>* self;
        void operator()(int v) const { self->apply_position(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::FlipView native_{nullptr};
    winrt::event_token                             selection_token_{};
    basic_carousel_view* bound_ = nullptr;
    bool                 suppress_ = false;

    items_cb_t items_cb_{this};
    pos_cb_t   pos_cb_{this};
    signal_slot<const std::vector<std::string>&> items_slot_{};
    signal_slot<const int&>                       pos_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_CAROUSEL_VIEW_HANDLER_HPP
