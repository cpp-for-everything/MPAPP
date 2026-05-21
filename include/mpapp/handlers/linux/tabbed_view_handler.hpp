// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 tabbed_view handler.
//
// Wraps a `GtkNotebook`. Each entry in `tab_titles` becomes a page
// whose tab is a `GtkLabel` with that text and whose page body is a
// trivial empty `GtkBox` placeholder (real page content lands when the
// `TabbedPage` page-level wiring arrives). `selected_index` maps
// directly to `gtk_notebook_set_current_page` (with -1 treated as "no
// selection" by clamping to no-op).
//
// The native GtkWidget exposed to dispatch surfaces is the outer
// GtkNotebook; the registrar at the bottom of the .cpp casts via
// `GTK_WIDGET()` for the registry callback. Per ADR-0013 the .cpp
// self-registers with `linux_dispatch`.

#ifndef MPAPP_HANDLERS_LINUX_TABBED_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_TABBED_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../tabbed_view.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class tabbed_view_handler<platform::linux_> {
public:
    tabbed_view_handler();
    ~tabbed_view_handler();

    tabbed_view_handler(const tabbed_view_handler&)            = delete;
    tabbed_view_handler& operator=(const tabbed_view_handler&) = delete;
    tabbed_view_handler(tabbed_view_handler&&)                 = delete;
    tabbed_view_handler& operator=(tabbed_view_handler&&)      = delete;

    void map_tab_titles(tabbed_view& t);
    void map_selected_index(tabbed_view& t);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_tab_titles(const std::vector<std::string>& v);
    void apply_selected_index(int v);

    struct tab_titles_cb_t     { tabbed_view_handler<platform::linux_>* self; void operator()(const std::vector<std::string>& v) const { self->apply_tab_titles(v); } };
    struct selected_index_cb_t { tabbed_view_handler<platform::linux_>* self; void operator()(int v) const { self->apply_selected_index(v); } };

    void* native_ = nullptr;  // GtkNotebook*
    bool  suppress_echo_ = false;

    tab_titles_cb_t                              tab_titles_cb_{this};
    selected_index_cb_t                          selected_index_cb_{this};
    signal_slot<std::vector<std::string> const&> tab_titles_slot_{};
    signal_slot<const int&>                      selected_index_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_TABBED_VIEW_HANDLER_HPP
