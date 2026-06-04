// SPDX-License-Identifier: Apache-2.0
// Android basic_absolute_layout handler. AbsoluteLayout maps to a custom
// FrameLayout subclass whose child placement is driven by absolute
// FrameLayout.LayoutParams leftMargin/topMargin + width/height (the
// framework-level android.widget.AbsoluteLayout is deprecated). Per-child
// placement reads the attached store (layout_bounds + layout_flags) on
// basic_absolute_layout; proportional flags are resolved against the
// container's measured width/height.

#ifndef MPAPP_HANDLERS_ANDROID_ABSOLUTE_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_ABSOLUTE_LAYOUT_HANDLER_HPP

#include "../../internal/basic_absolute_layout.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class absolute_layout_handler<platform::android> {
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

    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is
// pending the platform's real-handler task. No-op today
// so the wrapper ctor's unconditional
// `embedded_handler_.map_gestures(*this);` links.
void map_gestures(basic_absolute_layout& /*x*/) noexcept {}

private:
    // Resolve a child's attached rect against the container measured size
    // given its proportional flags, then set FrameLayout.LayoutParams.
    void apply_bounds(view& child, const rect& r, absolute_layout_flags f);

    jobject native_ = nullptr;  // custom absolute FrameLayout
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_ABSOLUTE_LAYOUT_HANDLER_HPP
