// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_tabbed_view handler implementation.

#include "mpapp/handlers/linux/tabbed_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

tabbed_view_handler<platform::linux_>::tabbed_view_handler() {
    GtkWidget* notebook = gtk_notebook_new();
    // Disable user re-order; the tab strip should reflect the model
    // verbatim. `scrollable` lets the strip overflow gracefully if the
    // caller pushes many tabs.
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
    native_ = notebook;
}

tabbed_view_handler<platform::linux_>::~tabbed_view_handler() = default;

void tabbed_view_handler<platform::linux_>::apply_tab_titles(const std::vector<std::string>& v) {
    if (native_ == nullptr || suppress_echo_) return;
    GtkNotebook* notebook = GTK_NOTEBOOK(static_cast<GtkWidget*>(native_));
    suppress_echo_ = true;

    // Clear existing pages — gtk_notebook_remove_page collapses the
    // index after every removal, so walk down from the end.
    int page_count = gtk_notebook_get_n_pages(notebook);
    for (int i = page_count - 1; i >= 0; --i) {
        gtk_notebook_remove_page(notebook, i);
    }

    // Append one basic_page per title. Each basic_page body is an empty
    // placeholder GtkBox — real content lands when the templating
    // engine ADR wires per-tab basic_page bodies. The tab basic_label is a
    // GtkLabel rendered on the tab strip.
    for (const auto& title : v) {
        GtkWidget* placeholder = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        GtkWidget* basic_label       = gtk_label_new(title.c_str());
        gtk_notebook_append_page(notebook, placeholder, basic_label);
    }

    suppress_echo_ = false;
}

void tabbed_view_handler<platform::linux_>::apply_selected_index(int v) {
    if (native_ == nullptr) return;
    GtkNotebook* notebook = GTK_NOTEBOOK(static_cast<GtkWidget*>(native_));
    if (v < 0) return;  // -1 ⇒ leave the notebook's own current basic_page

    const int page_count = gtk_notebook_get_n_pages(notebook);
    if (v >= page_count) return;  // out of range — no-op

    suppress_echo_ = true;
    gtk_notebook_set_current_page(notebook, v);
    suppress_echo_ = false;
}

void tabbed_view_handler<platform::linux_>::map_tab_titles(basic_tabbed_view& t) {
    apply_tab_titles(t.tab_titles.get());
    t.tab_titles.changed.subscribe(tab_titles_slot_, tab_titles_cb_);
}

void tabbed_view_handler<platform::linux_>::map_selected_index(basic_tabbed_view& t) {
    apply_selected_index(t.selected_index.get());
    t.selected_index.changed.subscribe(selected_index_slot_, selected_index_cb_);
}

} // namespace mpapp::internal
// ----- ADR-0013 self-registration --------------------------------------

namespace {

GtkWidget* dispatch_tabbed_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_tabbed_view*>(v); w && w->has_handler()) {
        return static_cast<GtkWidget*>(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_tabbed_view);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
