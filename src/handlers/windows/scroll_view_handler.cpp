// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 scroll_view handler implementation.

#include "mpapp/handlers/windows/scroll_view_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "mpapp/activity_indicator.hpp"
#include "mpapp/border.hpp"
#include "mpapp/box_view.hpp"
#include "mpapp/date_picker.hpp"
#include "mpapp/image.hpp"
#include "mpapp/image_button.hpp"
#include "mpapp/picker.hpp"
#include "mpapp/time_picker.hpp"
#include "mpapp/progress_bar.hpp"
#include "mpapp/search_bar.hpp"
#include "mpapp/button.hpp"
#include "mpapp/check_box.hpp"
#include "mpapp/editor.hpp"
#include "mpapp/entry.hpp"
#include "mpapp/handlers/windows/activity_indicator_handler.hpp"
#include "mpapp/handlers/windows/border_handler.hpp"
#include "mpapp/handlers/windows/box_view_handler.hpp"
#include "mpapp/handlers/windows/date_picker_handler.hpp"
#include "mpapp/handlers/windows/image_handler.hpp"
#include "mpapp/handlers/windows/image_button_handler.hpp"
#include "mpapp/handlers/windows/picker_handler.hpp"
#include "mpapp/handlers/windows/time_picker_handler.hpp"
#include "mpapp/handlers/windows/progress_bar_handler.hpp"
#include "mpapp/handlers/windows/search_bar_handler.hpp"
#include "mpapp/handlers/windows/button_handler.hpp"
#include "mpapp/handlers/windows/check_box_handler.hpp"
#include "mpapp/handlers/windows/editor_handler.hpp"
#include "mpapp/handlers/windows/entry_handler.hpp"
#include "mpapp/handlers/windows/label_handler.hpp"
#include "mpapp/handlers/windows/radio_button_handler.hpp"
#include "mpapp/handlers/windows/slider_handler.hpp"
#include "mpapp/handlers/windows/stack_layout_handler.hpp"
#include "mpapp/handlers/windows/stepper_handler.hpp"
#include "mpapp/handlers/windows/switch_handler.hpp"
#include "mpapp/label.hpp"
#include "mpapp/radio_button.hpp"
#include "mpapp/slider.hpp"
#include "mpapp/stack_layout.hpp"
#include "mpapp/stepper.hpp"
#include "mpapp/switch_.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

scroll_view_handler<platform::windows>::scroll_view_handler() {
    native_ = muxc::ScrollViewer{};
}

scroll_view_handler<platform::windows>::~scroll_view_handler() = default;

void scroll_view_handler<platform::windows>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    view* raw = v.get();
    if (raw == nullptr) {
        native_.Content(nullptr);
        return;
    }
    // ADR-0013: registry first.
    if (auto el = detail::windows_dispatch::dispatch(raw); el != nullptr) {
        native_.Content(el);
        return;
    }
    // All concrete widgets self-register via ADR-0013; the legacy
    // dynamic_cast chain has been removed.
}

void scroll_view_handler<platform::windows>::apply_orientation(scroll_orientation o) {
    if (native_ == nullptr) return;
    using muxc::ScrollMode;
    using muxc::ScrollBarVisibility;
    switch (o) {
        case scroll_orientation::vertical:
            native_.HorizontalScrollMode(ScrollMode::Disabled);
            native_.HorizontalScrollBarVisibility(ScrollBarVisibility::Hidden);
            native_.VerticalScrollMode(ScrollMode::Enabled);
            native_.VerticalScrollBarVisibility(ScrollBarVisibility::Auto);
            break;
        case scroll_orientation::horizontal:
            native_.HorizontalScrollMode(ScrollMode::Enabled);
            native_.HorizontalScrollBarVisibility(ScrollBarVisibility::Auto);
            native_.VerticalScrollMode(ScrollMode::Disabled);
            native_.VerticalScrollBarVisibility(ScrollBarVisibility::Hidden);
            break;
        case scroll_orientation::both:
            native_.HorizontalScrollMode(ScrollMode::Enabled);
            native_.HorizontalScrollBarVisibility(ScrollBarVisibility::Auto);
            native_.VerticalScrollMode(ScrollMode::Enabled);
            native_.VerticalScrollBarVisibility(ScrollBarVisibility::Auto);
            break;
        case scroll_orientation::neither:
            native_.HorizontalScrollMode(ScrollMode::Disabled);
            native_.VerticalScrollMode(ScrollMode::Disabled);
            break;
    }
}

void scroll_view_handler<platform::windows>::map_content(scroll_view& s) {
    bound_ = &s;
    apply_content(s.content.get());
    s.content.changed.subscribe(content_slot_, content_cb_);
}

void scroll_view_handler<platform::windows>::map_orientation(scroll_view& s) {
    apply_orientation(s.orientation.get());
    s.orientation.changed.subscribe(orient_slot_, orient_cb_);
}

void scroll_view_handler<platform::windows>::bind_content(scroll_view& s, view& child) {
    // Non-owning shared_ptr — the user owns the child's storage; the
    // shared_ptr just satisfies the Observable<shared_ptr<view>> contract.
    s.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp

// ---------- Self-registration with the per-platform dispatch registry --
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_scroll_view(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::scroll_view*>(v); s && s->has_handler()) {
        return s->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_scroll_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
