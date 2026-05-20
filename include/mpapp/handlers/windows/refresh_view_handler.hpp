// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 refresh_view handler.
//
// Composes a `mux::Controls::Grid` host with two cell-overlaid children:
//   - the wrapped scrollable content, and
//   - a `mux::Controls::ProgressRing` (visible + active iff is_refreshing).
//
// The Grid is the native UIElement exposed to dispatch surfaces. The
// MAUI-equivalent `mux::Controls::RefreshContainer` exists in WinAppSDK
// but its gesture/visualizer contract is heavier than the M-04b spike
// needs; the Grid+ProgressRing composition gives the same visible
// outcome with no risk of API drift across SDK versions.

#ifndef MPAPP_HANDLERS_WINDOWS_REFRESH_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_REFRESH_VIEW_HANDLER_HPP

#include <memory>

#include "../../platform.hpp"
#include "../../refresh_view.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class refresh_view_handler<platform::windows> {
public:
    refresh_view_handler();
    ~refresh_view_handler();

    refresh_view_handler(const refresh_view_handler&)            = delete;
    refresh_view_handler& operator=(const refresh_view_handler&) = delete;
    refresh_view_handler(refresh_view_handler&&)                 = delete;
    refresh_view_handler& operator=(refresh_view_handler&&)      = delete;

    void map_content(refresh_view& r);
    void map_is_refreshing(refresh_view& r);
    void map_refresh_color(refresh_view& r);

    void bind_content(refresh_view& r, view& child);

    // The host Grid IS the native UIElement exposed to dispatch surfaces.
    winrt::Microsoft::UI::Xaml::Controls::Grid&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Grid& native() const noexcept { return native_; }

private:
    void apply_content(const std::shared_ptr<view>& v);
    void apply_is_refreshing(bool v);
    void apply_refresh_color(const brush_ref& b);

    struct content_cb_t       { refresh_view_handler<platform::windows>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };
    struct is_refreshing_cb_t { refresh_view_handler<platform::windows>* self; void operator()(bool v) const { self->apply_is_refreshing(v); } };
    struct refresh_color_cb_t { refresh_view_handler<platform::windows>* self; void operator()(const brush_ref& b) const { self->apply_refresh_color(b); } };

    winrt::Microsoft::UI::Xaml::Controls::Grid          native_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::ProgressRing  spinner_{nullptr};

    content_cb_t                              content_cb_{this};
    is_refreshing_cb_t                        is_refreshing_cb_{this};
    refresh_color_cb_t                        refresh_color_cb_{this};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
    signal_slot<const bool&>                  is_refreshing_slot_{};
    signal_slot<const brush_ref&>             refresh_color_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_REFRESH_VIEW_HANDLER_HPP
