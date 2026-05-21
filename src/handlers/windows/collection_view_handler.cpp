// SPDX-License-Identifier: Apache-2.0
// WinUI 3 collection_view handler implementation.

#include "mpapp/handlers/windows/collection_view_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

collection_view_handler<platform::windows>::collection_view_handler() {
    native_ = muxc::ListView{};

    collection_view_handler* self = this;
    native_.SelectionChanged([self](
        winrt::Windows::Foundation::IInspectable const&,
        winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const&) {
        if (self->suppress_selection_event_) return;
        if (self->bound_ == nullptr) return;
        int new_idx = static_cast<int>(self->native_.SelectedIndex());
        if (self->bound_->selected_index.get() != new_idx) {
            self->bound_->selected_index.set(new_idx);
        }
        if (new_idx >= 0) self->bound_->item_tapped.emit(new_idx);
    });
}

collection_view_handler<platform::windows>::~collection_view_handler() = default;

void collection_view_handler<platform::windows>::rebuild_items(const std::vector<std::string>& v) {
    if (native_ == nullptr) return;
    suppress_selection_event_ = true;
    native_.Items().Clear();
    for (const auto& s : v) {
        native_.Items().Append(winrt::box_value(detail::to_hstring_utf8(s)));
    }
    if (bound_ != nullptr) apply_selection(bound_->selected_index.get());
    suppress_selection_event_ = false;
}

void collection_view_handler<platform::windows>::apply_selection(int idx) {
    if (native_ == nullptr) return;
    suppress_selection_event_ = true;
    if (idx < 0 || idx >= static_cast<int>(native_.Items().Size())) {
        native_.SelectedIndex(-1);
    } else {
        native_.SelectedIndex(idx);
    }
    suppress_selection_event_ = false;
}

void collection_view_handler<platform::windows>::apply_selection_mode(collection_selection_mode m) {
    if (native_ == nullptr) return;
    switch (m) {
        case collection_selection_mode::none:
            native_.SelectionMode(muxc::ListViewSelectionMode::None);
            break;
        case collection_selection_mode::multiple:
            native_.SelectionMode(muxc::ListViewSelectionMode::Multiple);
            break;
        case collection_selection_mode::single:
        default:
            native_.SelectionMode(muxc::ListViewSelectionMode::Single);
            break;
    }
}

void collection_view_handler<platform::windows>::map_items_source(collection_view& cv) {
    bound_ = &cv;
    rebuild_items(cv.items_source.get());
    cv.items_source.changed.subscribe(items_slot_, items_cb_);
}

void collection_view_handler<platform::windows>::map_selected_index(collection_view& cv) {
    apply_selection(cv.selected_index.get());
    cv.selected_index.changed.subscribe(sel_slot_, sel_cb_);
}

void collection_view_handler<platform::windows>::map_selection_mode(collection_view& cv) {
    apply_selection_mode(cv.selection_mode.get());
    cv.selection_mode.changed.subscribe(mode_slot_, mode_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_collection_view(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::collection_view*>(v); c && c->has_cv_handler()) {
        return c->cv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_collection_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
