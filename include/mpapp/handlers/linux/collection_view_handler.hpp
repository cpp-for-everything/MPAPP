// SPDX-License-Identifier: Apache-2.0
// GTK4 collection_view handler — same wrap-platform-recycler shape as
// list_view (GtkListBox in GtkScrolledWindow). Honors selection_mode
// via gtk_list_box_set_selection_mode.

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

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void rebuild_items(const std::vector<std::string>& v);
    void apply_selection(int idx);
    void apply_selection_mode(collection_selection_mode m);

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

    void* native_   = nullptr;  // GtkScrolledWindow*
    void* list_box_ = nullptr;  // GtkListBox*

    collection_view* bound_ = nullptr;

    items_cb_t items_cb_{this};
    sel_cb_t   sel_cb_{this};
    mode_cb_t  mode_cb_{this};
    signal_slot<const std::vector<std::string>&>          items_slot_{};
    signal_slot<const int&>                                sel_slot_{};
    signal_slot<const collection_selection_mode&>          mode_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_COLLECTION_VIEW_HANDLER_HPP
