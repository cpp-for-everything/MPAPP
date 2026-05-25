// SPDX-License-Identifier: Apache-2.0
// Android basic_collection_view handler. Outer FrameLayout (native_) is
// stable so the ADR-0013 dispatch handle doesn't move; inner_ is
// a single androidx.recyclerview.widget.RecyclerView whose
// LayoutManager swaps to cover all four collection_layout values:
//
//   * vertical_list   → LinearLayoutManager(VERTICAL)
//   * horizontal_list → LinearLayoutManager(HORIZONTAL)
//   * vertical_grid   → GridLayoutManager(span, VERTICAL)
//   * horizontal_grid → GridLayoutManager(span, HORIZONTAL)
//
// adapter_ is a long-lived MppCollectionAdapter instance attached to
// the RecyclerView once at construction; rebuild paths just call
// setStrings(...) or setNativeViews(...) on it. Selection state is
// owned by the adapter; multi-select pushes the full int[] back to
// native via MppItemClickRouter.nativeDispatchCheckedSet.

#ifndef MPAPP_HANDLERS_ANDROID_COLLECTION_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_COLLECTION_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../internal/basic_collection_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class collection_view_handler<platform::android> {
public:
    collection_view_handler();
    ~collection_view_handler();

    collection_view_handler(const collection_view_handler&)            = delete;
    collection_view_handler& operator=(const collection_view_handler&) = delete;
    collection_view_handler(collection_view_handler&&)                 = delete;
    collection_view_handler& operator=(collection_view_handler&&)      = delete;

    void map_items_source(basic_collection_view& cv);
    void map_typed_items(basic_collection_view& cv);
    void map_selected_index(basic_collection_view& cv);
    void map_selection_mode(basic_collection_view& cv);
    void map_layout(basic_collection_view& cv);

    jobject native() const noexcept { return native_; }

    // The RecyclerView itself — item_click_router doesn't need this
    // post-migration (MppCollectionAdapter calls native methods
    // directly), but it's kept as a convenience accessor for any
    // future router that wants to attach scroll listeners etc.
    jobject inner() const noexcept { return inner_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_collection_view& /*x*/) noexcept {}


private:
    void rebuild_items(const std::vector<std::string>& v);
    void rebuild_typed(const std::vector<view*>& v);
    void apply_selection(int idx);
    void apply_selection_mode(collection_selection_mode m);
    void apply_layout(collection_layout l);
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
    struct materialized_cb_t {
        collection_view_handler<platform::android>* self;
        void operator()() const { self->rebuild_active(); }
    };

    jobject native_  = nullptr;   // FrameLayout (outer)
    jobject inner_   = nullptr;   // androidx.recyclerview.widget.RecyclerView
    jobject adapter_ = nullptr;   // MppCollectionAdapter
    basic_collection_view* bound_ = nullptr;

    items_cb_t        items_cb_{this};
    typed_cb_t        typed_cb_{this};
    sel_cb_t          sel_cb_{this};
    mode_cb_t         mode_cb_{this};
    layout_cb_t       layout_cb_{this};
    materialized_cb_t materialized_cb_{this};
    signal_slot<const std::vector<std::string>&>          items_slot_{};
    signal_slot<const std::vector<view*>&>                typed_slot_{};
    signal_slot<const int&>                                sel_slot_{};
    signal_slot<const collection_selection_mode&>          mode_slot_{};
    signal_slot<const collection_layout&>                  layout_slot_{};
    signal_slot<>                                          materialized_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_COLLECTION_VIEW_HANDLER_HPP
