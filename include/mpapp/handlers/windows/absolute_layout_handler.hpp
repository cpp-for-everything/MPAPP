// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_absolute_layout handler. Wraps mux::Controls::Canvas, the
// closest native analogue to MAUI's AbsoluteLayout: children are placed
// via Canvas.SetLeft / Canvas.SetTop with explicit Width/Height. Per-child
// placement reads the attached store (layout_bounds + layout_flags) on
// basic_absolute_layout; proportional flags are resolved against the
// Canvas's current ActualWidth/ActualHeight.

#ifndef MPAPP_HANDLERS_WINDOWS_ABSOLUTE_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_ABSOLUTE_LAYOUT_HANDLER_HPP

#include "../../internal/basic_absolute_layout.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class absolute_layout_handler<platform::windows> {
public:
    absolute_layout_handler();
    ~absolute_layout_handler();

    absolute_layout_handler(const absolute_layout_handler&)            = delete;
    absolute_layout_handler& operator=(const absolute_layout_handler&) = delete;
    absolute_layout_handler(absolute_layout_handler&&)                 = delete;
    absolute_layout_handler& operator=(absolute_layout_handler&&)      = delete;

    // Map a child's attached layout_bounds / layout_flags onto the native
    // Canvas placement for that child's UIElement.
    void map_layout_bounds(basic_absolute_layout& a, view& child);
    void map_layout_flags(basic_absolute_layout& a, view& child);

    // Add a child at its attached (bounds, flags). The child's native
    // UIElement is resolved via the ADR-0013 dispatch registry.
    void add_child(basic_absolute_layout& a, view& child);

    winrt::Microsoft::UI::Xaml::Controls::Canvas&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Canvas& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is
// pending the platform's real-handler task. No-op today
// so the wrapper ctor's unconditional
// `embedded_handler_.map_gestures(*this);` links.
void map_gestures(basic_absolute_layout& /*x*/) noexcept {}

private:
    // Resolve a child's attached rect against the container extent given
    // its proportional flags, then push Canvas.SetLeft/SetTop + size.
    void apply_bounds(view& child, const rect& r, absolute_layout_flags f);

    winrt::Microsoft::UI::Xaml::Controls::Canvas native_{nullptr};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_ABSOLUTE_LAYOUT_HANDLER_HPP
