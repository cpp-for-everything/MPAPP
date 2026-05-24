// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_collection_view handler. Wraps the platform recycler inside a
// stable outer GtkScrolledWindow (native_) so the ADR-0013 dispatch
// handle doesn't move when we swap layouts.
//
//   * vertical_list   → GtkListBox  (single-column list, vertical scroll)
//   * horizontal_list → GtkBox      (single-row strip, horizontal scroll)
//   * vertical_grid   → GtkFlowBox  (orientation=HORIZONTAL, vertical scroll)
//   * horizontal_grid → GtkFlowBox  (orientation=VERTICAL,   horizontal scroll)
//
// The GtkScrolledWindow's hscrollbar/vscrollbar policy flips to match.
// The horizontal_list path uses GtkBox (not a 1-per-line GtkFlowBox)
// because FlowBox always allocates its natural size and may wrap
// regardless of max-children-per-line; GtkBox lays children along the
// axis at their natural widths and defers scrolling to the parent.
// Selection in horizontal_list is a v1 trade-off — GtkBox has no
// built-in selection, so selected_index does not visually highlight an
// item there; the property still tracks user clicks.

#ifndef MPAPP_HANDLERS_LINUX_COLLECTION_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_COLLECTION_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../internal/basic_collection_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class collection_view_handler<platform::linux_> {
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

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void rebuild_items(const std::vector<std::string>& v);
    void rebuild_typed(const std::vector<view*>& v);
    void apply_selection(int idx);
    void apply_selection_mode(collection_selection_mode m);
    void apply_layout(collection_layout l);
    void wire_tap_signals();
    void rebuild_active();

    struct items_cb_t {
        collection_view_handler<platform::linux_>* self;
        void operator()(const std::vector<std::string>&) const { self->rebuild_active(); }
    };
    struct typed_cb_t {
        collection_view_handler<platform::linux_>* self;
        void operator()(const std::vector<view*>&) const { self->rebuild_active(); }
    };
    struct sel_cb_t {
        collection_view_handler<platform::linux_>* self;
        void operator()(int v) const { self->apply_selection(v); }
    };
    struct mode_cb_t {
        collection_view_handler<platform::linux_>* self;
        void operator()(collection_selection_mode m) const { self->apply_selection_mode(m); }
    };
    struct layout_cb_t {
        collection_view_handler<platform::linux_>* self;
        void operator()(collection_layout l) const { self->apply_layout(l); }
    };
    struct materialized_cb_t {
        collection_view_handler<platform::linux_>* self;
        void operator()() const { self->rebuild_active(); }
    };

    // One value per collection_layout enum, plus an internal "unset"
    // sentinel used to force the first apply_layout call to actually
    // rebuild (the constructor seeds the inner widget but doesn't yet
    // know the surface's chosen layout).
    enum class layout_kind {
        unset,      // ctor default — first apply_layout always rebuilds
        list,       // vertical_list   — GtkListBox
        hbox,       // horizontal_list — GtkBox(HORIZONTAL)
        flow_horiz, // vertical_grid   — GtkFlowBox(HORIZONTAL)
        flow_vert,  // horizontal_grid — GtkFlowBox(VERTICAL)
    };

    void*       native_ = nullptr;          // GtkScrolledWindow*
    void*       inner_  = nullptr;          // active inner widget
    layout_kind kind_   = layout_kind::unset;

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
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_COLLECTION_VIEW_HANDLER_HPP
