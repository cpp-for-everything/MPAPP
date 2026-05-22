// SPDX-License-Identifier: Apache-2.0
// Android table_view handler. The outer widget is a FrameLayout that
// hosts one of two inner trees depending on which surface is in use:
//
//   * flat mode (sections non-empty, typed_sections empty):
//       FrameLayout
//         └── android.widget.ListView + ArrayAdapter<String>
//             rendering "▾ Section / row" entries
//
//   * typed mode (typed_sections non-empty):
//       FrameLayout
//         └── ScrollView
//             └── LinearLayout(VERTICAL)
//                 ├── TextView (section header, bold)
//                 ├── <cell native view> per row
//                 └── ... per section
//
// Switching between modes swaps the FrameLayout's child. The outer
// FrameLayout is the stable handle for ADR-0013 dispatch.

#ifndef MPAPP_HANDLERS_ANDROID_TABLE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_TABLE_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../table_view.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class table_view_handler<platform::android> {
public:
    table_view_handler();
    ~table_view_handler();

    table_view_handler(const table_view_handler&)            = delete;
    table_view_handler& operator=(const table_view_handler&) = delete;
    table_view_handler(table_view_handler&&)                 = delete;
    table_view_handler& operator=(table_view_handler&&)      = delete;

    void map_sections(table_view& tv);
    void map_typed_sections(table_view& tv);
    void map_row_height(table_view& tv);

    jobject native() const noexcept { return native_; }
    // Listed for the item_click_router so it can resolve the
    // tap-target ListView in flat mode (typed mode has its own
    // per-cell click routing built into each cell handler).
    jobject list_view() const noexcept { return list_view_; }

private:
    void rebuild_items(const std::vector<table_section_data>& sections);
    void rebuild_typed(const std::vector<table_section_typed>& sections);
    void apply_row_height(int h);
    void rebuild_active();

    struct sec_cb_t {
        table_view_handler<platform::android>* self;
        void operator()(const std::vector<table_section_data>&) const { self->rebuild_active(); }
    };
    struct typed_cb_t {
        table_view_handler<platform::android>* self;
        void operator()(const std::vector<table_section_typed>&) const { self->rebuild_active(); }
    };
    struct rh_cb_t {
        table_view_handler<platform::android>* self;
        void operator()(int v) const { self->apply_row_height(v); }
    };

    jobject native_     = nullptr;  // FrameLayout (outer)
    jobject list_view_  = nullptr;  // ListView (flat mode inner; nullptr in typed mode)
    jobject typed_root_ = nullptr;  // ScrollView (typed mode inner; nullptr in flat mode)
    table_view* bound_  = nullptr;

    sec_cb_t   sec_cb_{this};
    typed_cb_t typed_cb_{this};
    rh_cb_t    rh_cb_{this};
    signal_slot<const std::vector<table_section_data>&>  sec_slot_{};
    signal_slot<const std::vector<table_section_typed>&> typed_slot_{};
    signal_slot<const int&>                              rh_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_TABLE_VIEW_HANDLER_HPP
