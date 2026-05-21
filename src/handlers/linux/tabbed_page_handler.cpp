// SPDX-License-Identifier: Apache-2.0
// GTK4 tabbed_page handler implementation.

#include "mpapp/handlers/linux/tabbed_page_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/page.hpp"

namespace mpapp {

tabbed_page_handler<platform::linux_>::tabbed_page_handler() {
    native_ = gtk_notebook_new();
    gtk_widget_set_vexpand(static_cast<GtkWidget*>(native_), TRUE);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(native_), TRUE);
}

tabbed_page_handler<platform::linux_>::~tabbed_page_handler() = default;

void tabbed_page_handler<platform::linux_>::rebuild_children(const std::vector<page*>& kids) {
    GtkNotebook* nb = GTK_NOTEBOOK(static_cast<GtkWidget*>(native_));
    // Clear existing pages.
    while (gtk_notebook_get_n_pages(nb) > 0) {
        gtk_notebook_remove_page(nb, -1);
    }
    for (page* p : kids) {
        if (p == nullptr) continue;
        GtkWidget* child = detail::linux_dispatch::dispatch(p);
        if (child == nullptr) {
            // No native — append a placeholder GtkLabel so the tab still
            // shows up.
            child = gtk_label_new(p->title.get().c_str());
        }
        GtkWidget* tab_label = gtk_label_new(p->title.get().c_str());
        gtk_notebook_append_page(nb, child, tab_label);
    }
}

void tabbed_page_handler<platform::linux_>::apply_selection(int idx) {
    GtkNotebook* nb = GTK_NOTEBOOK(static_cast<GtkWidget*>(native_));
    if (idx < 0 || idx >= gtk_notebook_get_n_pages(nb)) return;
    gtk_notebook_set_current_page(nb, idx);
}

void tabbed_page_handler<platform::linux_>::map_children(tabbed_page& tp) {
    bound_ = &tp;
    rebuild_children(tp.children.get());
    tp.children.changed.subscribe(children_slot_, children_cb_);
}

void tabbed_page_handler<platform::linux_>::map_selected_index(tabbed_page& tp) {
    apply_selection(tp.selected_index.get());
    tp.selected_index.changed.subscribe(selection_slot_, selection_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_tabbed_page(::mpapp::view* v) {
    if (auto* t = dynamic_cast<::mpapp::tabbed_page*>(v); t && t->has_tp_handler()) {
        return GTK_WIDGET(t->tp_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_tabbed_page); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
