// SPDX-License-Identifier: Apache-2.0
// WinUI 3 collection_view handler implementation.
//
// native_ is a stable outer mux::Border so the ADR-0013 dispatch
// handle doesn't move when we swap layouts at runtime. inner_ is the
// active list-or-grid widget (mux::ListView for vertical_list,
// mux::GridView for vertical_grid).

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

namespace {

// Build a fresh inner list-or-grid widget for the given layout.
// vertical_list → mux::ListView; vertical_grid → mux::GridView.
// Horizontal modes degrade to their vertical counterpart in v1.
muxc::ListViewBase make_inner(collection_layout l) {
    switch (l) {
        case collection_layout::vertical_grid:
        case collection_layout::horizontal_grid:
            return muxc::GridView{};
        case collection_layout::horizontal_list:
        case collection_layout::vertical_list:
        default:
            return muxc::ListView{};
    }
}

muxc::ListViewSelectionMode to_native_mode(collection_selection_mode m) {
    switch (m) {
        case collection_selection_mode::none:     return muxc::ListViewSelectionMode::None;
        case collection_selection_mode::multiple: return muxc::ListViewSelectionMode::Multiple;
        case collection_selection_mode::single:
        default:                                  return muxc::ListViewSelectionMode::Single;
    }
}

} // namespace

collection_view_handler<platform::windows>::collection_view_handler() {
    native_ = muxc::Border{};
    inner_  = make_inner(collection_layout::vertical_list);
    native_.Child(inner_);
    wire_inner_selection_changed();
}

collection_view_handler<platform::windows>::~collection_view_handler() {
    if (inner_ != nullptr && selection_token_.value != 0) {
        try { inner_.SelectionChanged(selection_token_); } catch (...) {}
        selection_token_ = {};
    }
}

void collection_view_handler<platform::windows>::wire_inner_selection_changed() {
    if (inner_ == nullptr) return;
    if (selection_token_.value != 0) {
        try { inner_.SelectionChanged(selection_token_); } catch (...) {}
        selection_token_ = {};
    }
    auto* self = this;
    selection_token_ = inner_.SelectionChanged(
        [self](winrt::Windows::Foundation::IInspectable const&,
               muxc::SelectionChangedEventArgs const&) {
            if (self->suppress_selection_event_) return;
            if (self->bound_ == nullptr) return;

            int new_idx = static_cast<int>(self->inner_.SelectedIndex());
            if (self->bound_->selected_index.get() != new_idx) {
                self->bound_->selected_index.set(new_idx);
            }
            if (new_idx >= 0) self->bound_->item_tapped.emit(new_idx);

            if (self->bound_->selection_mode.get() == collection_selection_mode::multiple) {
                std::vector<int> idxs;
                auto items_view  = self->inner_.Items();
                auto selected_iv = self->inner_.SelectedItems();
                const uint32_t total = items_view.Size();
                const uint32_t sel_n = selected_iv.Size();
                idxs.reserve(sel_n);
                for (uint32_t i = 0; i < sel_n; ++i) {
                    uint32_t pos = 0;
                    if (items_view.IndexOf(selected_iv.GetAt(i), pos) && pos < total) {
                        idxs.push_back(static_cast<int>(pos));
                    }
                }
                if (self->bound_->selected_indices.get() != idxs) {
                    self->bound_->selected_indices.set(std::move(idxs));
                }
            }
        });
}

void collection_view_handler<platform::windows>::rebuild_items(const std::vector<std::string>& v) {
    if (inner_ == nullptr) return;
    suppress_selection_event_ = true;
    inner_.Items().Clear();
    for (const auto& s : v) {
        inner_.Items().Append(winrt::box_value(detail::to_hstring_utf8(s)));
    }
    if (bound_ != nullptr) apply_selection(bound_->selected_index.get());
    suppress_selection_event_ = false;
}

void collection_view_handler<platform::windows>::rebuild_typed(const std::vector<view*>& v) {
    if (inner_ == nullptr) return;
    suppress_selection_event_ = true;
    inner_.Items().Clear();
    for (view* item : v) {
        if (item == nullptr) continue;
        if (auto el = detail::windows_dispatch::dispatch(item); el != nullptr) {
            inner_.Items().Append(el);
        }
    }
    if (bound_ != nullptr) apply_selection(bound_->selected_index.get());
    suppress_selection_event_ = false;
}

void collection_view_handler<platform::windows>::rebuild_active() {
    if (bound_ == nullptr) return;
    if (!bound_->typed_items.get().empty()) {
        rebuild_typed(bound_->typed_items.get());
    } else {
        rebuild_items(bound_->items_source.get());
    }
}

void collection_view_handler<platform::windows>::apply_selection(int idx) {
    if (inner_ == nullptr) return;
    suppress_selection_event_ = true;
    if (idx < 0 || idx >= static_cast<int>(inner_.Items().Size())) {
        inner_.SelectedIndex(-1);
    } else {
        inner_.SelectedIndex(idx);
    }
    suppress_selection_event_ = false;
}

void collection_view_handler<platform::windows>::apply_selection_mode(collection_selection_mode m) {
    if (inner_ == nullptr) return;
    inner_.SelectionMode(to_native_mode(m));
}

void collection_view_handler<platform::windows>::apply_layout(collection_layout l) {
    if (native_ == nullptr) return;

    // If the requested layout maps to the same native class as the current
    // inner widget, skip the swap.
    const bool want_grid = (l == collection_layout::vertical_grid
                         || l == collection_layout::horizontal_grid);
    const bool have_grid = (inner_ != nullptr) && inner_.try_as<muxc::GridView>();
    if (inner_ != nullptr && want_grid == have_grid) return;

    // Detach old listener, then swap the inner widget under the Border.
    if (inner_ != nullptr && selection_token_.value != 0) {
        try { inner_.SelectionChanged(selection_token_); } catch (...) {}
        selection_token_ = {};
    }
    inner_ = make_inner(l);
    native_.Child(inner_);
    wire_inner_selection_changed();

    // Re-apply current state into the fresh widget.
    if (bound_ != nullptr) {
        apply_selection_mode(bound_->selection_mode.get());
        rebuild_active();
    }
}

void collection_view_handler<platform::windows>::map_items_source(collection_view& cv) {
    bound_ = &cv;
    rebuild_active();
    cv.items_source.changed.subscribe(items_slot_, items_cb_);
}

void collection_view_handler<platform::windows>::map_typed_items(collection_view& cv) {
    bound_ = &cv;
    rebuild_active();
    cv.typed_items.changed.subscribe(typed_slot_, typed_cb_);
}

void collection_view_handler<platform::windows>::map_selected_index(collection_view& cv) {
    apply_selection(cv.selected_index.get());
    cv.selected_index.changed.subscribe(sel_slot_, sel_cb_);
}

void collection_view_handler<platform::windows>::map_selection_mode(collection_view& cv) {
    apply_selection_mode(cv.selection_mode.get());
    cv.selection_mode.changed.subscribe(mode_slot_, mode_cb_);
}

void collection_view_handler<platform::windows>::map_layout(collection_view& cv) {
    apply_layout(cv.layout.get());
    cv.layout.changed.subscribe(layout_slot_, layout_cb_);
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
