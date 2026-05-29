// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/CarouselView.md
//
// `mpapp::internal::basic_carousel_view` — swipeable, paged item host.
// Counterpart to MAUI's `CarouselView` (the sibling of CollectionView).
// Mock surface (P2 / ADR-0008): `items_source` is a flat vector<string>
// stand-in (the rich items-source lands with the binding/template
// machinery); `position` is the current page; `loop` + `is_swipe_enabled`
// + `peek_count` mirror the MAUI bindables. `position_changed` is the
// PositionChanged event.
//
// Lands at `mock` status: surface + mock handler + tests. The wrapper
// (`mpapp::carousel_view`) + per-platform real handlers (WinUI FlipView,
// GTK4 Adw.Carousel / GtkStack, Android ViewPager2) are follow-ups, the
// same progression every control followed.

#ifndef MPAPP_INTERNAL_BASIC_CAROUSEL_VIEW_HPP
#define MPAPP_INTERNAL_BASIC_CAROUSEL_VIEW_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"
#include "../view.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class carousel_view_handler;

class basic_carousel_view : public view {
public:
    basic_carousel_view() = default;
    ~basic_carousel_view() override = default;

    basic_carousel_view(const basic_carousel_view&)            = delete;
    basic_carousel_view& operator=(const basic_carousel_view&) = delete;
    basic_carousel_view(basic_carousel_view&&)                 = delete;
    basic_carousel_view& operator=(basic_carousel_view&&)      = delete;

    // Flat-string items stand-in (rich items-source + DataTemplate land
    // with RFC-0007's DataTemplate follow-up).
    Observable<std::vector<std::string>> items_source{};

    // Current page index (MAUI's Position bindable, two-way).
    Observable<int>  position{ 0 };

    // Wrap past the ends (MAUI Loop, default true).
    Observable<bool> loop{ true };

    // Allow user swipe to change pages (MAUI IsSwipeEnabled).
    Observable<bool> is_swipe_enabled{ true };

    // Adjacent-item peek width hint (MAUI PeekAreaInsets, simplified to a
    // count of peeked neighbours for the mock).
    Observable<int>  peek_count{ 0 };

    // MAUI's PositionChanged event — fires when the settled page changes.
    mpapp::signal<int> position_changed{};

    [[nodiscard]] std::size_t item_count() const { return items_source.get().size(); }

    // Programmatic page change. Wraps when `loop` is set and the target
    // is out of range; otherwise clamps. Fires position_changed only on
    // a real change.
    void scroll_to(int index) {
        const int n = static_cast<int>(item_count());
        int target = index;
        if (n > 0) {
            if (loop.get()) {
                target = ((index % n) + n) % n;   // wrap
            } else if (target < 0) {
                target = 0;
            } else if (target >= n) {
                target = n - 1;
            }
        }
        if (target == position.get()) {
            return;
        }
        position.set(target);
        position_changed.emit(target);
    }

    // ---- Handler ------------------------------------------------------
    carousel_view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const carousel_view_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                            has_handler() const noexcept { return handler_ != nullptr; }
    void                                            set_handler(carousel_view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    carousel_view_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal

#endif // MPAPP_INTERNAL_BASIC_CAROUSEL_VIEW_HPP
