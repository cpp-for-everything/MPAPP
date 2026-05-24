// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_list_view handler implementation.

#include "mpapp/handlers/windows/list_view_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

namespace mpapp::internal {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

list_view_handler<platform::windows>::list_view_handler() {
    native_ = muxc::ListView{};
    native_.SelectionMode(muxc::ListViewSelectionMode::Single);

    // Wire SelectionChanged -> bound_->selected_index. Skip the event
    // when WE caused the change to avoid an infinite loop.
    list_view_handler* self = this;
    native_.SelectionChanged([self](
        winrt::Windows::Foundation::IInspectable const&,
        winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const&) {
        if (self->suppress_selection_event_) return;
        if (self->bound_ == nullptr) return;
        int new_idx = static_cast<int>(self->native_.SelectedIndex());
        if (self->bound_->selected_index.get() != new_idx) {
            self->bound_->selected_index.set(new_idx);
        }
        if (new_idx >= 0) {
            self->bound_->item_tapped.emit(new_idx);
        }
    });
}

list_view_handler<platform::windows>::~list_view_handler() = default;

void list_view_handler<platform::windows>::rebuild_items(const std::vector<std::string>& v) {
    if (native_ == nullptr) return;
    suppress_selection_event_ = true;
    native_.Items().Clear();
    for (const auto& s : v) {
        native_.Items().Append(winrt::box_value(detail::to_hstring_utf8(s)));
    }
    // Restore selection if still valid.
    if (bound_ != nullptr) apply_selection(bound_->selected_index.get());
    suppress_selection_event_ = false;
}

void list_view_handler<platform::windows>::apply_selection(int idx) {
    if (native_ == nullptr) return;
    suppress_selection_event_ = true;
    if (idx < 0 || idx >= static_cast<int>(native_.Items().Size())) {
        native_.SelectedIndex(-1);
    } else {
        native_.SelectedIndex(idx);
    }
    suppress_selection_event_ = false;
}

void list_view_handler<platform::windows>::map_items_source(basic_list_view& lv) {
    bound_ = &lv;
    rebuild_items(lv.items_source.get());
    lv.items_source.changed.subscribe(items_slot_, items_cb_);
}

void list_view_handler<platform::windows>::map_selected_index(basic_list_view& lv) {
    apply_selection(lv.selected_index.get());
    lv.selected_index.changed.subscribe(sel_slot_, sel_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_list_view(::mpapp::view* v) {
    if (auto* l = dynamic_cast<::mpapp::internal::basic_list_view*>(v); l && l->has_lv_handler()) {
        return l->lv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_list_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
