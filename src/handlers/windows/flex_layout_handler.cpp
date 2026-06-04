// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_flex_layout handler implementation. WinUI has no built-in
// flexbox panel, so this hosts the children in a mux::Controls::Canvas and
// computes their rectangles with the platform-neutral mpapp::flex_arrange
// solver (the same algorithm used everywhere), pushing the results via
// Canvas.SetLeft/SetTop + Width/Height. A single SizeChanged hook re-solves
// on resize; container-property changes + add_child re-solve too.

#include "mpapp/handlers/windows/flex_layout_handler.hpp"

#if defined(_WIN32)

#include <cmath>
#include <vector>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/layout/flex_arrange.hpp"
#include "mpapp/handlers/windows/widget_dispatch.hpp"

namespace mpapp::internal {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

flex_layout_handler<platform::windows>::flex_layout_handler() {
    native_ = muxc::Canvas{};
}

flex_layout_handler<platform::windows>::~flex_layout_handler() {
    if (native_ != nullptr && size_token_.value != 0) {
        native_.SizeChanged(size_token_);
    }
}

// ----- container-property mappers: bind + (re)solve --------------------------

void flex_layout_handler<platform::windows>::map_direction(basic_flex_layout& f) {
    bound_ = &f;
    f.direction.changed.subscribe(direction_slot_, direction_cb_);
    relayout();
}
void flex_layout_handler<platform::windows>::map_wrap(basic_flex_layout& f) {
    bound_ = &f;
    f.wrap.changed.subscribe(wrap_slot_, wrap_cb_);
    relayout();
}
void flex_layout_handler<platform::windows>::map_justify_content(basic_flex_layout& f) {
    bound_ = &f;
    f.justify_content.changed.subscribe(justify_slot_, justify_cb_);
    relayout();
}
void flex_layout_handler<platform::windows>::map_align_items(basic_flex_layout& f) {
    bound_ = &f;
    f.align_items.changed.subscribe(align_items_slot_, align_items_cb_);
    relayout();
}
void flex_layout_handler<platform::windows>::map_align_content(basic_flex_layout& f) {
    bound_ = &f;
    f.align_content.changed.subscribe(align_content_slot_, align_content_cb_);
    relayout();
}
void flex_layout_handler<platform::windows>::map_position(basic_flex_layout& f) {
    bound_ = &f;
    f.position.changed.subscribe(position_slot_, position_cb_);
    relayout();
}

// Property-change callbacks just re-solve (relayout reads the live surface).
void flex_layout_handler<platform::windows>::apply_direction(flex_direction)         { relayout(); }
void flex_layout_handler<platform::windows>::apply_wrap(flex_wrap)                    { relayout(); }
void flex_layout_handler<platform::windows>::apply_justify_content(flex_justify)      { relayout(); }
void flex_layout_handler<platform::windows>::apply_align_items(flex_align_items)      { relayout(); }
void flex_layout_handler<platform::windows>::apply_align_content(flex_align_content)  { relayout(); }
void flex_layout_handler<platform::windows>::apply_position(flex_position)            { relayout(); }

void flex_layout_handler<platform::windows>::add_child(basic_flex_layout& f, view& child) {
    bound_ = &f;
    if (native_ == nullptr) {
        return;
    }
    auto el = detail::windows_dispatch::dispatch(&child);
    if (el != nullptr) {
        native_.Children().Append(el);
    }
    if (!size_hooked_) {
        size_token_ = native_.SizeChanged(
            [this](winrt::Windows::Foundation::IInspectable const&,
                   mux::SizeChangedEventArgs const&) { relayout(); });
        size_hooked_ = true;
    }
    relayout();
}

// ----- the solver wiring -----------------------------------------------------

void flex_layout_handler<platform::windows>::relayout() {
    if (native_ == nullptr || bound_ == nullptr) {
        return;
    }

    const double cw = native_.ActualWidth();
    const double ch = native_.ActualHeight();

    ::mpapp::flex_container_input ci{};
    ci.width           = cw;
    ci.height          = ch;
    ci.direction       = bound_->direction.get();
    ci.wrap            = bound_->wrap.get();
    ci.justify_content = bound_->justify_content.get();
    ci.align_items     = bound_->align_items.get();
    ci.align_content   = bound_->align_content.get();

    const bool row = ::mpapp::internal::flex::is_row(ci.direction);
    // Measure children with the available container extent (fall back to a
    // large size before the first real layout pass) so DesiredSize is sane.
    const float avail_w = (cw > 0.0) ? static_cast<float>(cw) : 100000.0f;
    const float avail_h = (ch > 0.0) ? static_cast<float>(ch) : 100000.0f;

    std::vector<::mpapp::flex_item_input> items;
    std::vector<mux::UIElement>           els;
    const std::size_t n = bound_->child_count();
    items.reserve(n);
    els.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        view* c = bound_->child_at(i);
        if (c == nullptr) {
            continue;
        }
        auto el = detail::windows_dispatch::dispatch(c);
        if (el == nullptr) {
            continue;
        }
        const auto cp = bound_->get_child_props(*c);

        ::mpapp::flex_item_input it{};
        it.basis      = cp.basis;
        it.grow       = cp.grow;
        it.shrink     = cp.shrink;
        it.align_self = cp.align_self;

        el.Measure({avail_w, avail_h});
        const auto ds = el.DesiredSize();
        it.measured_main  = row ? ds.Width  : ds.Height;
        it.measured_cross = row ? ds.Height : ds.Width;

        items.push_back(it);
        els.push_back(el);
    }

    const auto rects = ::mpapp::flex_arrange(ci, items);
    for (std::size_t i = 0; i < els.size() && i < rects.size(); ++i) {
        muxc::Canvas::SetLeft(els[i], rects[i].x);
        muxc::Canvas::SetTop(els[i], rects[i].y);
        if (auto fe = els[i].try_as<mux::FrameworkElement>()) {
            if (rects[i].width  > 0.0) { fe.Width(rects[i].width); }
            if (rects[i].height > 0.0) { fe.Height(rects[i].height); }
        }
    }
}

} // namespace mpapp::internal

#endif // _WIN32
