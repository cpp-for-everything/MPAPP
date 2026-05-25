// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_flyout_view handler.
//
// Wraps `mux::Controls::NavigationView` — the WinUI 3 control whose
// Content / PaneCustomContent / IsPaneOpen surfaces directly mirror
// `mpapp::basic_flyout_view`'s `detail` / `flyout` / `is_presented`.
//
// Per ADR-0013 the .cpp self-registers with `windows_dispatch`. The
// NavigationView itself is a `UIElement` (via `Control` → `FrameworkElement`),
// so returning `native_` from the dispatcher requires no cast.

#ifndef MPAPP_HANDLERS_WINDOWS_FLYOUT_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_FLYOUT_VIEW_HANDLER_HPP

#include <memory>

#include "../../internal/basic_flyout_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class flyout_view_handler<platform::windows> {
public:
    flyout_view_handler();
    ~flyout_view_handler();

    flyout_view_handler(const flyout_view_handler&)            = delete;
    flyout_view_handler& operator=(const flyout_view_handler&) = delete;
    flyout_view_handler(flyout_view_handler&&)                 = delete;
    flyout_view_handler& operator=(flyout_view_handler&&)      = delete;

    void map_flyout(basic_flyout_view& f);
    void map_detail(basic_flyout_view& f);
    void map_is_presented(basic_flyout_view& f);

    void bind_flyout(basic_flyout_view& f, view& child);
    void bind_detail(basic_flyout_view& f, view& child);

    // The host NavigationView IS the native UIElement exposed to dispatch
    // surfaces.
    winrt::Microsoft::UI::Xaml::Controls::NavigationView&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::NavigationView& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_flyout_view& /*x*/) noexcept {}


private:
    void apply_flyout(const std::shared_ptr<view>& v);
    void apply_detail(const std::shared_ptr<view>& v);
    void apply_is_presented(bool v);

    struct flyout_cb_t       { flyout_view_handler<platform::windows>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_flyout(v); } };
    struct detail_cb_t       { flyout_view_handler<platform::windows>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_detail(v); } };
    struct is_presented_cb_t { flyout_view_handler<platform::windows>* self; void operator()(bool v) const { self->apply_is_presented(v); } };

    winrt::Microsoft::UI::Xaml::Controls::NavigationView native_{nullptr};

    flyout_cb_t                               flyout_cb_{this};
    detail_cb_t                               detail_cb_{this};
    is_presented_cb_t                         is_presented_cb_{this};
    signal_slot<std::shared_ptr<view> const&> flyout_slot_{};
    signal_slot<std::shared_ptr<view> const&> detail_slot_{};
    signal_slot<const bool&>                  is_presented_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_FLYOUT_VIEW_HANDLER_HPP
