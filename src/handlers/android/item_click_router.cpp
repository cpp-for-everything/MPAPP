// SPDX-License-Identifier: Apache-2.0
// Native side of MppItemClickRouter — dispatches AdapterView.onItemClick
// events to list_view / collection_view observables.

#if defined(__ANDROID__)

#include <jni.h>

#include "mpapp/cell.hpp"
#include "mpapp/collection_view.hpp"
#include "mpapp/handlers/android/collection_view_handler.hpp"
#include "mpapp/list_view.hpp"
#include "mpapp/table_view.hpp"

namespace mpapp::detail {

enum class item_click_kind : int {
    list_view       = 0,
    collection_view = 1,
    table_view      = 2,
};

// Decode a flat ListView position (section-header + section-data rows
// concatenated) back into (section, row). Returns true and populates
// `out_section` / `out_row` if `position` lands on a data row; returns
// false (and silently swallows) if `position` is a section title row.
static bool decode_table_position(const table_view& tv,
                                  int position,
                                  int& out_section,
                                  int& out_row) {
    int idx = position;
    const auto& sections = tv.sections.get();
    for (std::size_t s = 0; s < sections.size(); ++s) {
        if (idx == 0) {
            // Section title row — not a tap target.
            return false;
        }
        idx -= 1; // consume the section title row
        const int rows_in_section = static_cast<int>(sections[s].rows.size());
        if (idx < rows_in_section) {
            out_section = static_cast<int>(s);
            out_row     = idx;
            return true;
        }
        idx -= rows_in_section;
    }
    return false;
}

void dispatch_android_item_click(jlong owner_ptr, jint kind, jint position) {
    switch (static_cast<item_click_kind>(kind)) {
        case item_click_kind::list_view: {
            auto* lv = reinterpret_cast<list_view*>(owner_ptr);
            if (lv == nullptr) return;
            if (lv->selected_index.get() != static_cast<int>(position)) {
                lv->selected_index.set(static_cast<int>(position));
            }
            lv->item_tapped.emit(static_cast<int>(position));
            break;
        }
        case item_click_kind::collection_view: {
            auto* cv = reinterpret_cast<collection_view*>(owner_ptr);
            if (cv == nullptr) return;
            if (cv->selected_index.get() != static_cast<int>(position)) {
                cv->selected_index.set(static_cast<int>(position));
            }
            cv->item_tapped.emit(static_cast<int>(position));
            // In multi-select mode, ListView has already toggled the
            // checked state for the tapped row by the time we get here.
            // Pull the full set out via getCheckedItemPositions() and
            // mirror it into selected_indices.
            if (cv->selection_mode.get() == collection_selection_mode::multiple
                && cv->has_cv_handler()) {
                cv->cv_handler().refresh_multi_selection_from_native();
            }
            break;
        }
        case item_click_kind::table_view: {
            auto* tv = reinterpret_cast<table_view*>(owner_ptr);
            if (tv == nullptr) return;
            int section = 0, row = 0;
            if (decode_table_position(*tv, static_cast<int>(position), section, row)) {
                tv->row_tapped.emit(section, row);
                // Cross-platform cell-tapped routing: when typed_sections
                // is populated, look up the cell at this coordinate and
                // bubble the tap into its own `tapped` signal.
                if (cell* c = tv->cell_at(section, row); c != nullptr) {
                    c->tapped.emit();
                }
            }
            break;
        }
    }
}

} // namespace mpapp::detail

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppItemClickRouter_nativeDispatchItemClick(
    JNIEnv* /*env*/,
    jclass  /*cls*/,
    jlong   owner_ptr,
    jint    kind,
    jint    position) {
    mpapp::detail::dispatch_android_item_click(owner_ptr, kind, position);
}

#endif // __ANDROID__
