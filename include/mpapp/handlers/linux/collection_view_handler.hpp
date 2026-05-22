// SPDX-License-Identifier: Apache-2.0
// GTK4 collection_view handler. Wraps the platform recycler inside a
// stable outer GtkScrolledWindow (native_) so the ADR-0013 dispatch
// handle doesn't move when we swap layouts.
//
//   * vertical_list  → GtkListBox  (single-column list w/ selection)
//   * vertical_grid  → GtkFlowBox  (wrapping multi-column grid w/ selection)
//
// Horizontal modes degrade to their vertical counterpart in v1.

#ifndef MPAPP_HANDLERS_LINUX_COLLECTION_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_COLLECTION_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../collection_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class collection_view_handler<platform::linux_> {
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
    void map_layout(collection_view& cv);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void rebuild_items(const std::vector<std::string>& v);
    void apply_selection(int idx);
    void apply_selection_mode(collection_selection_mode m);
    void apply_layout(collection_layout l);
    void wire_tap_signals();

    struct items_cb_t {
        collection_view_handler<platform::linux_>* self;
        void operator()(const std::vector<std::string>& v) const { self->rebuild_items(v); }
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

    void* native_   = nullptr;  // GtkScrolledWindow*
    void* inner_    = nullptr;  // GtkListBox* (list) or GtkFlowBox* (grid)
    bool  is_grid_  = false;    // true iff inner_ is a GtkFlowBox

    collection_view* bound_ = nullptr;

    items_cb_t  items_cb_{this};
    sel_cb_t    sel_cb_{this};
    mode_cb_t   mode_cb_{this};
    layout_cb_t layout_cb_{this};
    signal_slot<const std::vector<std::string>&>          items_slot_{};
    signal_slot<const int&>                                sel_slot_{};
    signal_slot<const collection_selection_mode&>          mode_slot_{};
    signal_slot<const collection_layout&>                  layout_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_COLLECTION_VIEW_HANDLER_HPP
