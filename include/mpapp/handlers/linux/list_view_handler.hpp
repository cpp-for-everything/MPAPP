// SPDX-License-Identifier: Apache-2.0
// GTK4 list_view handler. Uses GtkListBox + per-item GtkLabel rows.
// Wrapped in a GtkScrolledWindow so the list scrolls when long.

#ifndef MPAPP_HANDLERS_LINUX_LIST_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_LIST_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../list_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class list_view_handler<platform::linux_> {
public:
    list_view_handler();
    ~list_view_handler();

    list_view_handler(const list_view_handler&)            = delete;
    list_view_handler& operator=(const list_view_handler&) = delete;
    list_view_handler(list_view_handler&&)                 = delete;
    list_view_handler& operator=(list_view_handler&&)      = delete;

    void map_items_source(list_view& lv);
    void map_selected_index(list_view& lv);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void rebuild_items(const std::vector<std::string>& v);
    void apply_selection(int idx);

    struct items_cb_t {
        list_view_handler<platform::linux_>* self;
        void operator()(const std::vector<std::string>& v) const { self->rebuild_items(v); }
    };
    struct sel_cb_t {
        list_view_handler<platform::linux_>* self;
        void operator()(int v) const { self->apply_selection(v); }
    };

    void* native_   = nullptr;  // GtkScrolledWindow*
    void* list_box_ = nullptr;  // GtkListBox* (child of scrolled window)

    list_view* bound_ = nullptr;
    bool       suppress_row_selected_ = false;

    items_cb_t items_cb_{this};
    sel_cb_t   sel_cb_{this};
    signal_slot<const std::vector<std::string>&> items_slot_{};
    signal_slot<const int&>                       sel_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_LIST_VIEW_HANDLER_HPP
