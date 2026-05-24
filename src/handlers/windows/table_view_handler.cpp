// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_table_view handler implementation.

#include "mpapp/handlers/windows/table_view_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

#include "mpapp/cell.hpp"
#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "winrt_strings.hpp"

namespace mpapp::internal {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

namespace {

// Decode a flat ListView position (section-header row + data rows
// concatenated, same shape as rebuild_items / rebuild_typed) back to
// (section, row). Returns true if `position` lands on a data row;
// false if it's a header (or out of range).
//
// Template-duck-typed on whichever section vector the basic_table_view has
// populated — both `table_section_data` and `table_section_typed`
// expose `.rows.size()`.
template <class SectionVec>
bool decode_position(const SectionVec& sections,
                     int position,
                     int& out_section,
                     int& out_row) {
    int idx = position;
    for (std::size_t s = 0; s < sections.size(); ++s) {
        if (idx == 0) return false;             // header row — not a tap target
        idx -= 1;
        const int rows_in = static_cast<int>(sections[s].rows.size());
        if (idx < rows_in) {
            out_section = static_cast<int>(s);
            out_row     = idx;
            return true;
        }
        idx -= rows_in;
    }
    return false;
}

} // namespace

table_view_handler<platform::windows>::table_view_handler() {
    native_ = muxc::ListView{};
    native_.SelectionMode(muxc::ListViewSelectionMode::Single);

    // Wire row taps via SelectionChanged. We immediately deselect after
    // emitting so the row doesn't stay highlighted — matches MAUI's
    // TableView semantics.
    auto* self = this;
    native_.SelectionChanged([self](
        winrt::Windows::Foundation::IInspectable const&,
        muxc::SelectionChangedEventArgs const&) {
        if (self->bound_ == nullptr) return;
        const int idx = static_cast<int>(self->native_.SelectedIndex());
        if (idx < 0) return;

        int section = 0, row = 0;
        const auto& typed = self->bound_->typed_sections.get();
        const bool ok = !typed.empty()
            ? decode_position(typed, idx, section, row)
            : decode_position(self->bound_->sections.get(), idx, section, row);
        // Clear selection so taps don't stick.
        self->native_.SelectedIndex(-1);
        if (!ok) return;

        self->bound_->row_tapped.emit(section, row);
        if (cell* c = self->bound_->cell_at(section, row); c != nullptr) {
            c->tapped.emit();
        }
    });
}

table_view_handler<platform::windows>::~table_view_handler() = default;

void table_view_handler<platform::windows>::rebuild_items(const std::vector<table_section_data>& sections) {
    if (native_ == nullptr) return;
    native_.Items().Clear();
    for (const auto& sec : sections) {
        // Section header — non-selectable in v1, styled by a leading "▾ " marker.
        std::string header = "▾ " + sec.title;
        native_.Items().Append(winrt::box_value(detail::to_hstring_utf8(header)));
        for (const auto& row : sec.rows) {
            native_.Items().Append(winrt::box_value(detail::to_hstring_utf8(row)));
        }
    }
}

void table_view_handler<platform::windows>::rebuild_typed(const std::vector<table_section_typed>& sections) {
    if (native_ == nullptr) return;
    native_.Items().Clear();
    for (const auto& sec : sections) {
        // Section header — still a plain bold-marked string for v1.
        std::string header = "▾ " + sec.title;
        native_.Items().Append(winrt::box_value(detail::to_hstring_utf8(header)));
        for (cell* c : sec.rows) {
            if (c == nullptr) continue;
            auto el = detail::windows_dispatch::dispatch(c);
            if (el != nullptr) {
                native_.Items().Append(el);
            }
        }
    }
}

void table_view_handler<platform::windows>::rebuild_active() {
    if (bound_ == nullptr) return;
    const auto& typed = bound_->typed_sections.get();
    if (!typed.empty()) {
        rebuild_typed(typed);
    } else {
        rebuild_items(bound_->sections.get());
    }
}

void table_view_handler<platform::windows>::apply_row_height(int /*h*/) {
    // row_height honoring is tied to ItemContainerStyle customization;
    // not wired in v1.
}

void table_view_handler<platform::windows>::map_sections(basic_table_view& tv) {
    bound_ = &tv;
    rebuild_active();
    tv.sections.changed.subscribe(sec_slot_, sec_cb_);
}

void table_view_handler<platform::windows>::map_typed_sections(basic_table_view& tv) {
    bound_ = &tv;
    rebuild_active();
    tv.typed_sections.changed.subscribe(typed_slot_, typed_cb_);
}

void table_view_handler<platform::windows>::map_row_height(basic_table_view& tv) {
    apply_row_height(tv.row_height.get());
    tv.row_height.changed.subscribe(rh_slot_, rh_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_table_view(::mpapp::view* v) {
    if (auto* t = dynamic_cast<::mpapp::internal::basic_table_view*>(v); t && t->has_tv_handler()) {
        return t->tv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_table_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
