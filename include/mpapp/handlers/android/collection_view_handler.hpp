// SPDX-License-Identifier: Apache-2.0
// Android collection_view handler. Same wrap pattern as list_view —
// android.widget.ListView + ArrayAdapter<String>. setChoiceMode honors
// the cross-platform selection_mode enum (None/Single/Multiple). Multi
// mode echoes the full checked-item set back into selected_indices
// after each tap via refresh_multi_selection_from_native().

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
    void map_selected_index(collection_view& cv);
    void map_selection_mode(collection_view& cv);

    jobject native() const noexcept { return native_; }

    // Invoked by item_click_router after each multi-mode tap. Reads
    // ListView.getCheckedItemPositions() and writes the indices vector
    // into the bound collection_view.
    void refresh_multi_selection_from_native();

private:
    void rebuild_items(const std::vector<std::string>& v);
    void apply_selection(int idx);
    void apply_selection_mode(collection_selection_mode m);

    struct items_cb_t {
        collection_view_handler<platform::android>* self;
        void operator()(const std::vector<std::string>& v) const { self->rebuild_items(v); }
    };
    struct sel_cb_t {
        collection_view_handler<platform::android>* self;
        void operator()(int v) const { self->apply_selection(v); }
    };
    struct mode_cb_t {
        collection_view_handler<platform::android>* self;
        void operator()(collection_selection_mode m) const { self->apply_selection_mode(m); }
    };

    jobject native_ = nullptr;
    collection_view* bound_ = nullptr;

    items_cb_t items_cb_{this};
    sel_cb_t   sel_cb_{this};
    mode_cb_t  mode_cb_{this};
    signal_slot<const std::vector<std::string>&>          items_slot_{};
    signal_slot<const int&>                                sel_slot_{};
    signal_slot<const collection_selection_mode&>          mode_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_COLLECTION_VIEW_HANDLER_HPP
