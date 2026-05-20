// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 indicator_view handler — renders a row of dots
// manually using a horizontal `GtkBox` of small `GtkLabel` widgets whose
// per-instance CSS class provides a circular background-color.
//
// No native page-indicator widget exists in GTK4. The handler rebuilds
// the box children whenever `count` changes and reloads its single CSS
// provider whenever `position` / `indicator_color` /
// `selected_indicator_color` change. Per ADR-0013 the .cpp self-registers
// with `linux_dispatch`.

#ifndef MPAPP_HANDLERS_LINUX_INDICATOR_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_INDICATOR_VIEW_HANDLER_HPP

#include <string>

#include "../../indicator_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class indicator_view_handler<platform::linux_> {
public:
    indicator_view_handler();
    ~indicator_view_handler();

    indicator_view_handler(const indicator_view_handler&)            = delete;
    indicator_view_handler& operator=(const indicator_view_handler&) = delete;

    void map_count(indicator_view& iv);
    void map_position(indicator_view& iv);
    void map_indicator_color(indicator_view& iv);
    void map_selected_indicator_color(indicator_view& iv);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void rebuild_dots();
    void reload_css();

    void apply_count(int v);
    void apply_position(int v);
    void apply_indicator_color(const brush_ref& b);
    void apply_selected_indicator_color(const brush_ref& b);

    struct count_cb_t    { indicator_view_handler<platform::linux_>* self; void operator()(int v) const { self->apply_count(v); } };
    struct position_cb_t { indicator_view_handler<platform::linux_>* self; void operator()(int v) const { self->apply_position(v); } };
    struct color_cb_t    { indicator_view_handler<platform::linux_>* self; void operator()(const brush_ref& b) const { self->apply_indicator_color(b); } };
    struct sel_color_cb_t{ indicator_view_handler<platform::linux_>* self; void operator()(const brush_ref& b) const { self->apply_selected_indicator_color(b); } };

    void*       native_   = nullptr;   // GtkBox*
    void*       provider_ = nullptr;   // GtkCssProvider*
    std::string base_class_{};         // "mpapp-iv-<n>" — shared by all dots

    int        cached_count_    = 0;
    int        cached_position_ = 0;
    brush_ref  cached_color_{};
    brush_ref  cached_selected_{};

    count_cb_t                    count_cb_{this};
    position_cb_t                 position_cb_{this};
    color_cb_t                    color_cb_{this};
    sel_color_cb_t                sel_color_cb_{this};
    signal_slot<const int&>       count_slot_{};
    signal_slot<const int&>       position_slot_{};
    signal_slot<const brush_ref&> color_slot_{};
    signal_slot<const brush_ref&> sel_color_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_INDICATOR_VIEW_HANDLER_HPP
