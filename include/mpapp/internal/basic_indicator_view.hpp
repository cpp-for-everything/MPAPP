// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/IndicatorView.md
//
// `mpapp::indicator_view` — a row of small dots showing how many items are
// in a paged collection and which one is currently selected. Presentational
// only; the host wires `count` to the paired collection's item count and
// `position` to the currently visible index.
//
// No native page-indicator widget exists on any of the three supported
// runtime platforms. Each handler renders the row manually:
//   - Windows: a horizontal `mux::Controls::StackPanel` of
//     `mux::Shapes::Ellipse` instances.
//   - Linux:   a horizontal `GtkBox` of GTK widgets with CSS-styled
//     background-color dots.
//   - Android: a horizontal `LinearLayout` of `View`s whose background is
//     a circular `GradientDrawable`.

#ifndef MPAPP_INTERNAL_BASIC_INDICATOR_VIEW_HPP
#define MPAPP_INTERNAL_BASIC_INDICATOR_VIEW_HPP

#include "../observable.hpp"
#include "../platform.hpp"
#include "../view.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class indicator_view_handler;

class basic_indicator_view : public view {
public:
    basic_indicator_view() = default;

    Observable<int>        count{0};                       // number of dots
    Observable<int>        position{0};                    // highlighted dot index
    Observable<brush_ref>  indicator_color{};              // unselected dot tint
    Observable<brush_ref>  selected_indicator_color{};     // selected dot tint

    indicator_view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const indicator_view_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                             has_handler() const noexcept { return handler_ != nullptr; }
    void                                             set_handler(indicator_view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    indicator_view_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_INDICATOR_VIEW_HPP
