// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — WinUI 3 basic_stack_layout handler implementation.

#include "mpapp/handlers/windows/stack_layout_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "mpapp/internal/basic_activity_indicator.hpp"
#include "mpapp/border.hpp"
#include "mpapp/internal/basic_box_view.hpp"
#include "mpapp/internal/basic_date_picker.hpp"
#include "mpapp/internal/basic_image.hpp"
#include "mpapp/internal/basic_image_button.hpp"
#include "mpapp/internal/basic_time_picker.hpp"
#include "mpapp/internal/basic_picker.hpp"
#include "mpapp/internal/basic_progress_bar.hpp"
#include "mpapp/internal/basic_search_bar.hpp"
#include "mpapp/internal/basic_button.hpp"
#include "mpapp/internal/basic_check_box.hpp"
#include "mpapp/editor.hpp"
#include "mpapp/internal/basic_entry.hpp"
#include "mpapp/handlers/windows/activity_indicator_handler.hpp"
#include "mpapp/handlers/windows/border_handler.hpp"
#include "mpapp/handlers/windows/box_view_handler.hpp"
#include "mpapp/handlers/windows/date_picker_handler.hpp"
#include "mpapp/handlers/windows/image_handler.hpp"
#include "mpapp/handlers/windows/image_button_handler.hpp"
#include "mpapp/handlers/windows/time_picker_handler.hpp"
#include "mpapp/handlers/windows/picker_handler.hpp"
#include "mpapp/handlers/windows/progress_bar_handler.hpp"
#include "mpapp/handlers/windows/search_bar_handler.hpp"
#include "mpapp/handlers/windows/button_handler.hpp"
#include "mpapp/handlers/windows/check_box_handler.hpp"
#include "mpapp/handlers/windows/editor_handler.hpp"
#include "mpapp/handlers/windows/entry_handler.hpp"
#include "mpapp/handlers/windows/label_handler.hpp"
#include "mpapp/handlers/windows/radio_button_handler.hpp"
#include "mpapp/handlers/windows/slider_handler.hpp"
#include "mpapp/handlers/windows/stepper_handler.hpp"
#include "mpapp/handlers/windows/switch_handler.hpp"
#include "mpapp/internal/basic_label.hpp"
#include "mpapp/internal/basic_radio_button.hpp"
#include "mpapp/internal/basic_slider.hpp"
#include "mpapp/internal/basic_stepper.hpp"
#include "mpapp/internal/basic_switch_.hpp"

namespace mpapp::internal {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

stack_layout_handler<platform::windows>::stack_layout_handler() {
    native_ = muxc::StackPanel{};
}

stack_layout_handler<platform::windows>::~stack_layout_handler() = default;

void stack_layout_handler<platform::windows>::apply_orientation(orientation o) {
    if (native_ == nullptr) {
        return;
    }
    native_.Orientation(o == orientation::horizontal
                            ? muxc::Orientation::Horizontal
                            : muxc::Orientation::Vertical);
}

void stack_layout_handler<platform::windows>::apply_spacing(double s) {
    if (native_ != nullptr) {
        native_.Spacing(s);
    }
}

void stack_layout_handler<platform::windows>::apply_padding(thickness t) {
    if (native_ != nullptr) {
        native_.Padding(mux::Thickness{t.left, t.top, t.right, t.bottom});
    }
}

namespace {

mux::HorizontalAlignment to_native(h_align a) noexcept {
    switch (a) {
        case h_align::start:   return mux::HorizontalAlignment::Left;
        case h_align::center:  return mux::HorizontalAlignment::Center;
        case h_align::end:     return mux::HorizontalAlignment::Right;
        case h_align::stretch: return mux::HorizontalAlignment::Stretch;
    }
    return mux::HorizontalAlignment::Stretch;
}

mux::VerticalAlignment to_native(v_align a) noexcept {
    switch (a) {
        case v_align::start:   return mux::VerticalAlignment::Top;
        case v_align::center:  return mux::VerticalAlignment::Center;
        case v_align::end:     return mux::VerticalAlignment::Bottom;
        case v_align::stretch: return mux::VerticalAlignment::Stretch;
    }
    return mux::VerticalAlignment::Stretch;
}

} // namespace

void stack_layout_handler<platform::windows>::apply_horizontal_alignment(h_align a) {
    if (native_ != nullptr) {
        native_.HorizontalAlignment(to_native(a));
    }
}

void stack_layout_handler<platform::windows>::apply_vertical_alignment(v_align a) {
    if (native_ != nullptr) {
        native_.VerticalAlignment(to_native(a));
    }
}

void stack_layout_handler<platform::windows>::bind(basic_stack_layout& s) {
    bound_ = &s;

    apply_orientation(s.stack_orientation.get());
    s.stack_orientation.changed.subscribe(orient_slot_, orient_cb_);

    apply_spacing(s.spacing.get());
    s.spacing.changed.subscribe(spacing_slot_, spacing_cb_);

    apply_padding(s.padding.get());
    s.padding.changed.subscribe(padding_slot_, padding_cb_);

    apply_horizontal_alignment(s.horizontal_alignment.get());
    s.horizontal_alignment.changed.subscribe(h_align_slot_, h_align_cb_);

    apply_vertical_alignment(s.vertical_alignment.get());
    s.vertical_alignment.changed.subscribe(v_align_slot_, v_align_cb_);

    // Initial children — the user has typically already populated the
    // layout via `add(...)` before binding. Replay them into the
    // native panel.
    for (std::size_t i = 0; i < s.child_count(); ++i) {
        if (view* child = s.child_at(i); child != nullptr) {
            add_child(*child);
        }
    }
}

void stack_layout_handler<platform::windows>::add_child(view& child) {
    if (native_ == nullptr) {
        return;
    }
    // ADR-0013: registry first.
    if (auto el = detail::windows_dispatch::dispatch(&child); el != nullptr) {
        native_.Children().Append(el);
        return;
    }
    // Unknown subtype — silently drop. All concrete widgets now self-register
    // via ADR-0013, so an unknown subtype here means a handler that has not
    // been built into the link target.
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_stack_layout(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::internal::basic_stack_layout*>(v); s && s->has_handler()) {
        return s->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_stack_layout); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
