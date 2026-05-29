// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_carousel_view handler implementation.

#include "mpapp/handlers/windows/carousel_view_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/handlers/windows/gesture_attach.hpp"

#include "winrt_strings.hpp"

namespace mpapp::internal {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

carousel_view_handler<platform::windows>::carousel_view_handler() {
    native_ = muxc::FlipView{};
    wire_selection_changed();
}

carousel_view_handler<platform::windows>::~carousel_view_handler() {
    if (native_ != nullptr && selection_token_.value != 0) {
        try { native_.SelectionChanged(selection_token_); } catch (...) {}
        selection_token_ = {};
    }
}

void carousel_view_handler<platform::windows>::wire_selection_changed() {
    if (native_ == nullptr) return;
    auto* self = this;
    selection_token_ = native_.SelectionChanged(
        [self](winrt::Windows::Foundation::IInspectable const&,
               muxc::SelectionChangedEventArgs const&) {
            if (self->suppress_) return;
            if (self->bound_ == nullptr) return;
            const int new_idx = static_cast<int>(self->native_.SelectedIndex());
            if (self->bound_->position.get() != new_idx) {
                self->bound_->position.set(new_idx);
                self->bound_->position_changed.emit(new_idx);
            }
        });
}

void carousel_view_handler<platform::windows>::rebuild_items(const std::vector<std::string>& v) {
    if (native_ == nullptr) return;
    suppress_ = true;
    native_.Items().Clear();
    for (const auto& s : v) {
        native_.Items().Append(winrt::box_value(detail::to_hstring_utf8(s)));
    }
    if (bound_ != nullptr) apply_position(bound_->position.get());
    suppress_ = false;
}

void carousel_view_handler<platform::windows>::apply_position(int idx) {
    if (native_ == nullptr) return;
    suppress_ = true;
    if (idx < 0 || idx >= static_cast<int>(native_.Items().Size())) {
        native_.SelectedIndex(-1);
    } else {
        native_.SelectedIndex(idx);
    }
    suppress_ = false;
}

void carousel_view_handler<platform::windows>::map_items_source(basic_carousel_view& c) {
    bound_ = &c;
    rebuild_items(c.items_source.get());
    c.items_source.changed.subscribe(items_slot_, items_cb_);
}

void carousel_view_handler<platform::windows>::map_position(basic_carousel_view& c) {
    apply_position(c.position.get());
    c.position.changed.subscribe(pos_slot_, pos_cb_);
}

void carousel_view_handler<platform::windows>::map_loop(basic_carousel_view& /*c*/) {
    // Loop/clamp handled in basic_carousel_view::scroll_to; FlipView has no
    // native loop surface (it stops at the ends on user swipe).
}

void carousel_view_handler<platform::windows>::map_is_swipe_enabled(basic_carousel_view& /*c*/) {
    // FlipView swipe is always touch-enabled in WinUI 3 — no public
    // IsSwipeEnabled. v1 no-op (gating swipe would need a custom
    // ManipulationMode pass).
}

void carousel_view_handler<platform::windows>::map_peek_count(basic_carousel_view& /*c*/) {
    // FlipView shows exactly one item — peek is a v1 no-op.
}

void carousel_view_handler<platform::windows>::map_gestures(basic_carousel_view& c) {
    windows_gestures::attach(native_, c);
}

} // namespace mpapp::internal

// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_carousel_view(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::internal::basic_carousel_view*>(v);
        c && c->has_handler()) {
        return c->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_carousel_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
