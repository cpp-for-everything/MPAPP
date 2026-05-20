// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — WinUI 3 stack_layout handler implementation.

#include "mpapp/handlers/windows/stack_layout_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "mpapp/activity_indicator.hpp"
#include "mpapp/border.hpp"
#include "mpapp/box_view.hpp"
#include "mpapp/date_picker.hpp"
#include "mpapp/image.hpp"
#include "mpapp/image_button.hpp"
#include "mpapp/time_picker.hpp"
#include "mpapp/picker.hpp"
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
#include "mpapp/label.hpp"
#include "mpapp/radio_button.hpp"
#include "mpapp/slider.hpp"
#include "mpapp/stepper.hpp"
#include "mpapp/switch_.hpp"

namespace mpapp {

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

void stack_layout_handler<platform::windows>::bind(stack_layout& s) {
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
    if (auto* b = dynamic_cast<button*>(&child); b != nullptr) {
        if (b->has_handler()) {
            native_.Children().Append(b->handler().native());
        }
        return;
    }
    if (auto* l = dynamic_cast<label*>(&child); l != nullptr) {
        if (l->has_handler()) {
            native_.Children().Append(l->handler().native());
        }
        return;
    }
    if (auto* sl = dynamic_cast<stack_layout*>(&child); sl != nullptr) {
        if (sl->has_handler()) {
            native_.Children().Append(sl->handler().native());
        }
        return;
    }
    if (auto* e = dynamic_cast<entry*>(&child); e != nullptr) {
        if (e->has_handler()) {
            native_.Children().Append(e->handler().native());
        }
        return;
    }
    if (auto* sw = dynamic_cast<switch_*>(&child); sw != nullptr) {
        if (sw->has_handler()) {
            native_.Children().Append(sw->handler().native());
        }
        return;
    }
    if (auto* cb = dynamic_cast<check_box*>(&child); cb != nullptr) {
        if (cb->has_handler()) {
            native_.Children().Append(cb->handler().native());
        }
        return;
    }
    if (auto* rb = dynamic_cast<radio_button*>(&child); rb != nullptr) {
        if (rb->has_handler()) {
            native_.Children().Append(rb->handler().native());
        }
        return;
    }
    if (auto* sl = dynamic_cast<slider*>(&child); sl != nullptr) {
        if (sl->has_handler()) {
            native_.Children().Append(sl->handler().native());
        }
        return;
    }
    if (auto* st = dynamic_cast<stepper*>(&child); st != nullptr) {
        if (st->has_handler()) {
            native_.Children().Append(st->handler().native());
        }
        return;
    }
    if (auto* ed = dynamic_cast<editor*>(&child); ed != nullptr) {
        if (ed->has_handler()) {
            native_.Children().Append(ed->handler().native());
        }
        return;
    }
    if (auto* bx = dynamic_cast<box_view*>(&child); bx != nullptr) {
        if (bx->has_handler()) {
            native_.Children().Append(bx->handler().native());
        }
        return;
    }
    if (auto* br = dynamic_cast<border*>(&child); br != nullptr) {
        if (br->has_handler()) {
            native_.Children().Append(br->handler().native());
        }
        return;
    }
    if (auto* ai = dynamic_cast<activity_indicator*>(&child); ai != nullptr) {
        if (ai->has_handler()) {
            native_.Children().Append(ai->handler().native());
        }
        return;
    }
    if (auto* pb = dynamic_cast<progress_bar*>(&child); pb != nullptr) {
        if (pb->has_handler()) {
            native_.Children().Append(pb->handler().native());
        }
        return;
    }
    if (auto* sb = dynamic_cast<search_bar*>(&child); sb != nullptr) {
        if (sb->has_handler()) {
            native_.Children().Append(sb->handler().native());
        }
        return;
    }
    if (auto* pk = dynamic_cast<picker*>(&child); pk != nullptr) {
        if (pk->has_handler()) {
            native_.Children().Append(pk->handler().native());
        }
        return;
    }
    if (auto* dp = dynamic_cast<date_picker*>(&child); dp != nullptr) {
        if (dp->has_handler()) {
            native_.Children().Append(dp->handler().native());
        }
        return;
    }
    if (auto* tp = dynamic_cast<time_picker*>(&child); tp != nullptr) {
        if (tp->has_handler()) {
            native_.Children().Append(tp->handler().native());
        }
        return;
    }
    if (auto* im = dynamic_cast<image*>(&child); im != nullptr) {
        if (im->has_handler()) {
            native_.Children().Append(im->handler().native());
        }
        return;
    }
    if (auto* ib = dynamic_cast<image_button*>(&child); ib != nullptr) {
        if (ib->has_handler()) {
            native_.Children().Append(ib->handler().native());
        }
        return;
    }
    // Unknown subtype — silently drop. As more handlers ship, append
    // new branches here.
}

} // namespace mpapp

#endif // _WIN32
