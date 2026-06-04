// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_absolute_layout handler. GtkFixed is the native analogue to
// MAUI's AbsoluteLayout: children are placed at explicit (x, y) via
// gtk_fixed_put / gtk_fixed_move, and sized via gtk_widget_set_size_request.
//
// Note: GtkFixed has no notion of proportional placement. Proportional
// layout_flags are resolved by mpapp against the container's current
// allocation (width/height) before calling gtk_fixed_move — a future
// follow-up could re-resolve on the GtkFixed "size-allocate" signal so
// proportional children track container resizes.

#ifndef MPAPP_HANDLERS_LINUX_ABSOLUTE_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_ABSOLUTE_LAYOUT_HANDLER_HPP

#include "../../internal/basic_absolute_layout.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class absolute_layout_handler<platform::linux_> {
public:
    absolute_layout_handler();
    ~absolute_layout_handler();

    absolute_layout_handler(const absolute_layout_handler&)            = delete;
    absolute_layout_handler& operator=(const absolute_layout_handler&) = delete;
    absolute_layout_handler(absolute_layout_handler&&)                 = delete;
    absolute_layout_handler& operator=(absolute_layout_handler&&)      = delete;

    void map_layout_bounds(basic_absolute_layout& a, view& child);
    void map_layout_flags(basic_absolute_layout& a, view& child);

    void add_child(basic_absolute_layout& a, view& child);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs
// matching GtkGesture* controllers via
// `mpapp::internal::linux_gestures::attach`.
void map_gestures(basic_absolute_layout& x);

private:
    // Resolve a child's attached rect against the container allocation
    // given its proportional flags, then gtk_fixed_move + size_request.
    void apply_bounds(view& child, const rect& r, absolute_layout_flags f);

    void* native_ = nullptr;  // GtkFixed*
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_ABSOLUTE_LAYOUT_HANDLER_HPP
