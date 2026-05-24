// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_indicator_view handler — renders a row of dots
// manually using a horizontal `mux::Controls::StackPanel` of
// `mux::Shapes::Ellipse` children.
//
// No native basic_page-indicator widget exists in WinUI 3. The handler rebuilds
// the ellipse list whenever `count` changes; on `position` /
// `indicator_color` / `selected_indicator_color` change it walks the
// existing children and re-tints them in place. Per ADR-0013 the .cpp
// self-registers with `windows_dispatch`.

#ifndef MPAPP_HANDLERS_WINDOWS_INDICATOR_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_INDICATOR_VIEW_HANDLER_HPP

#include "../../internal/basic_indicator_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class indicator_view_handler<platform::windows> {
public:
    indicator_view_handler();
    ~indicator_view_handler();

    indicator_view_handler(const indicator_view_handler&)            = delete;
    indicator_view_handler& operator=(const indicator_view_handler&) = delete;
    indicator_view_handler(indicator_view_handler&&)                 = delete;
    indicator_view_handler& operator=(indicator_view_handler&&)      = delete;

    void map_count(basic_indicator_view& iv);
    void map_position(basic_indicator_view& iv);
    void map_indicator_color(basic_indicator_view& iv);
    void map_selected_indicator_color(basic_indicator_view& iv);

    winrt::Microsoft::UI::Xaml::Controls::StackPanel&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::StackPanel& native() const noexcept { return native_; }

private:
    void rebuild_dots();
    void recolor_dots();

    void apply_count(int v);
    void apply_position(int v);
    void apply_indicator_color(const brush_ref& b);
    void apply_selected_indicator_color(const brush_ref& b);

    struct count_cb_t    { indicator_view_handler<platform::windows>* self; void operator()(int v) const { self->apply_count(v); } };
    struct position_cb_t { indicator_view_handler<platform::windows>* self; void operator()(int v) const { self->apply_position(v); } };
    struct color_cb_t    { indicator_view_handler<platform::windows>* self; void operator()(const brush_ref& b) const { self->apply_indicator_color(b); } };
    struct sel_color_cb_t{ indicator_view_handler<platform::windows>* self; void operator()(const brush_ref& b) const { self->apply_selected_indicator_color(b); } };

    winrt::Microsoft::UI::Xaml::Controls::StackPanel native_{nullptr};

    basic_indicator_view*               bound_      = nullptr;
    int                           cached_count_    = 0;
    int                           cached_position_ = 0;
    brush_ref                     cached_color_{};
    brush_ref                     cached_selected_{};

    count_cb_t                    count_cb_{this};
    position_cb_t                 position_cb_{this};
    color_cb_t                    color_cb_{this};
    sel_color_cb_t                sel_color_cb_{this};
    signal_slot<const int&>       count_slot_{};
    signal_slot<const int&>       position_slot_{};
    signal_slot<const brush_ref&> color_slot_{};
    signal_slot<const brush_ref&> sel_color_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_INDICATOR_VIEW_HANDLER_HPP
