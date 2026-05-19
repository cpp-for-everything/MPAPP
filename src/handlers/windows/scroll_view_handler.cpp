// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 scroll_view handler implementation.

#include "mpapp/handlers/windows/scroll_view_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/button.hpp"
#include "mpapp/check_box.hpp"
#include "mpapp/editor.hpp"
#include "mpapp/entry.hpp"
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
    // Dispatch on the known concrete subtypes (mirrors window_handler).
    if (auto* sl = dynamic_cast<stack_layout*>(raw); sl && sl->has_handler()) {
        native_.Content(sl->handler().native()); return;
    }
    if (auto* b = dynamic_cast<button*>(raw); b && b->has_handler()) {
        native_.Content(b->handler().native()); return;
    }
    if (auto* l = dynamic_cast<label*>(raw); l && l->has_handler()) {
        native_.Content(l->handler().native()); return;
    }
    if (auto* e = dynamic_cast<entry*>(raw); e && e->has_handler()) {
        native_.Content(e->handler().native()); return;
    }
    if (auto* sw = dynamic_cast<switch_*>(raw); sw && sw->has_handler()) {
        native_.Content(sw->handler().native()); return;
    }
    if (auto* cb = dynamic_cast<check_box*>(raw); cb && cb->has_handler()) {
        native_.Content(cb->handler().native()); return;
    }
    if (auto* rb = dynamic_cast<radio_button*>(raw); rb && rb->has_handler()) {
        native_.Content(rb->handler().native()); return;
    }
    if (auto* sl2 = dynamic_cast<slider*>(raw); sl2 && sl2->has_handler()) {
        native_.Content(sl2->handler().native()); return;
    }
    if (auto* st = dynamic_cast<stepper*>(raw); st && st->has_handler()) {
        native_.Content(st->handler().native()); return;
    }
    if (auto* ed = dynamic_cast<editor*>(raw); ed && ed->has_handler()) {
        native_.Content(ed->handler().native()); return;
    }
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

#endif // _WIN32
