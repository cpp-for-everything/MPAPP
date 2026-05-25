// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_grid_layout handler implementation.

#include "mpapp/handlers/linux/grid_layout_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

grid_layout_handler<platform::linux_>::grid_layout_handler() {
    native_ = gtk_grid_new();
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(native_), TRUE);
    gtk_widget_set_vexpand(static_cast<GtkWidget*>(native_), TRUE);
}

grid_layout_handler<platform::linux_>::~grid_layout_handler() = default;

void grid_layout_handler<platform::linux_>::apply_row_spacing(double s) {
    if (native_ == nullptr) return;
    gtk_grid_set_row_spacing(GTK_GRID(static_cast<GtkWidget*>(native_)),
                             static_cast<guint>(s < 0 ? 0 : s));
}

void grid_layout_handler<platform::linux_>::apply_column_spacing(double s) {
    if (native_ == nullptr) return;
    gtk_grid_set_column_spacing(GTK_GRID(static_cast<GtkWidget*>(native_)),
                                static_cast<guint>(s < 0 ? 0 : s));
}

void grid_layout_handler<platform::linux_>::rebuild_dummy_rows(const std::vector<track_def>& /*v*/) {
    // GtkGrid sizes tracks implicitly. row_definitions are advisory on
    // Linux for now — see header note. Star/Auto behavior comes from
    // child hexpand/vexpand flags, which the handler applies on attach.
}

void grid_layout_handler<platform::linux_>::rebuild_dummy_columns(const std::vector<track_def>& /*v*/) {
    // Same as rebuild_dummy_rows.
}

void grid_layout_handler<platform::linux_>::add_child(basic_grid_layout& g, view& child) {
    if (native_ == nullptr) return;
    GtkWidget* w = detail::linux_dispatch::dispatch(&child);
    if (w == nullptr) return;

    const auto p = g.get_placement(child);
    gtk_grid_attach(GTK_GRID(static_cast<GtkWidget*>(native_)),
                    w,
                    p.column, p.row,
                    p.column_span, p.row_span);

    // Honor Star tracks by setting expand flags on the child when its
    // column/row sits on a Star track.
    const auto& cols = g.column_definitions.get();
    const auto& rows = g.row_definitions.get();
    if (p.column >= 0 && static_cast<std::size_t>(p.column) < cols.size()
        && cols[static_cast<std::size_t>(p.column)].k == track_def::kind::star) {
        gtk_widget_set_hexpand(w, TRUE);
    }
    if (p.row >= 0 && static_cast<std::size_t>(p.row) < rows.size()
        && rows[static_cast<std::size_t>(p.row)].k == track_def::kind::star) {
        gtk_widget_set_vexpand(w, TRUE);
    }
}

void grid_layout_handler<platform::linux_>::map_row_definitions(basic_grid_layout& g) {
    rebuild_dummy_rows(g.row_definitions.get());
    g.row_definitions.changed.subscribe(rows_slot_, rows_cb_);
}

void grid_layout_handler<platform::linux_>::map_column_definitions(basic_grid_layout& g) {
    rebuild_dummy_columns(g.column_definitions.get());
    g.column_definitions.changed.subscribe(cols_slot_, cols_cb_);
}

void grid_layout_handler<platform::linux_>::map_row_spacing(basic_grid_layout& g) {
    apply_row_spacing(g.row_spacing.get());
    g.row_spacing.changed.subscribe(rsp_slot_, rsp_cb_);
}

void grid_layout_handler<platform::linux_>::map_column_spacing(basic_grid_layout& g) {
    apply_column_spacing(g.column_spacing.get());
    g.column_spacing.changed.subscribe(csp_slot_, csp_cb_);
}

void grid_layout_handler<platform::linux_>::map_gestures(basic_grid_layout& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_grid_layout(::mpapp::view* v) {
    if (auto* g = dynamic_cast<::mpapp::internal::basic_grid_layout*>(v); g && g->has_handler()) {
        return GTK_WIDGET(g->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_grid_layout); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
