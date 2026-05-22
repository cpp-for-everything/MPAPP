// SPDX-License-Identifier: Apache-2.0
// Android collection_view handler. Outer FrameLayout (native_) is
// stable so the ADR-0013 dispatch handle doesn't move; inner_ is the
// active list-or-grid widget:
//   * vertical_list → android.widget.ListView (single column)
//   * vertical_grid → android.widget.GridView (auto-column wrap)
//
// Both inherit from AdapterView so setChoiceMode + checked-position
// query semantics are uniform — refresh_multi_selection_from_native()
// works against either.

#ifndef MPAPP_HANDLERS_ANDROID_COLLECTION_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_COLLECTION_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../collection_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class collection_view_handler<platform::android> {
public:
    collection_view_handler();
    ~collection_view_handler();

    collection_view_handler(const collection_view_handler&)            = delete;
    collection_view_handler& operator=(const collection_view_handler&) = delete;
    collection_view_handler(collection_view_handler&&)                 = delete;
    collection_view_handler& operator=(collection_view_handler&&)      = delete;

    void map_items_source(collection_view& cv);
    void map_typed_items(collection_view& cv);
    void map_selected_index(collection_view& cv);
    void map_selection_mode(collection_view& cv);
    void map_layout(collection_view& cv);

    jobject native() const noexcept { return native_; }

    // Invoked by item_click_router after each multi-mode tap. Reads
    // ListView/GridView.getCheckedItemPositions() and writes the
    // indices vector into the bound collection_view.
    void refresh_multi_selection_from_native();

    // The list-or-grid inner widget; item_click_router uses this to
    // resolve which AdapterView fired the event.
    jobject inner() const noexcept { return inner_; }

private:
    void rebuild_items(const std::vector<std::string>& v);
    void rebuild_typed(const std::vector<view*>& v);
    void apply_selection(int idx);
    void apply_selection_mode(collection_selection_mode m);
    void apply_layout(collection_layout l);
    void rebuild_inner_for_layout(collection_layout l);
    void rebuild_active();

    struct items_cb_t {
        collection_view_handler<platform::android>* self;
        void operator()(const std::vector<std::string>&) const { self->rebuild_active(); }
    };
    struct typed_cb_t {
        collection_view_handler<platform::android>* self;
        void operator()(const std::vector<view*>&) const { self->rebuild_active(); }
    };
    struct sel_cb_t {
        collection_view_handler<platform::android>* self;
        void operator()(int v) const { self->apply_selection(v); }
    };
    struct mode_cb_t {
        collection_view_handler<platform::android>* self;
        void operator()(collection_selection_mode m) const { self->apply_selection_mode(m); }
    };
    struct layout_cb_t {
        collection_view_handler<platform::android>* self;
        void operator()(collection_layout l) const { self->apply_layout(l); }
    };

    jobject native_  = nullptr;   // FrameLayout (outer)
    jobject inner_   = nullptr;   // ListView or GridView (whichever matches layout)
    bool    is_grid_ = false;
    collection_view* bound_ = nullptr;

    items_cb_t  items_cb_{this};
    typed_cb_t  typed_cb_{this};
    sel_cb_t    sel_cb_{this};
    mode_cb_t   mode_cb_{this};
    layout_cb_t layout_cb_{this};
    signal_slot<const std::vector<std::string>&>          items_slot_{};
    signal_slot<const std::vector<view*>&>                typed_slot_{};
    signal_slot<const int&>                                sel_slot_{};
    signal_slot<const collection_selection_mode&>          mode_slot_{};
    signal_slot<const collection_layout&>                  layout_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_COLLECTION_VIEW_HANDLER_HPP
