// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_collection_view handler implementation.

#include "mpapp/handlers/linux/collection_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

void set_suppress(void* inner, bool on) {
    if (inner == nullptr) return;
    g_object_set_data(G_OBJECT(static_cast<GtkWidget*>(inner)),
                      "mpapp_suppress",
                      on ? GINT_TO_POINTER(1) : nullptr);
}

bool is_suppressed(void* inner) {
    return inner != nullptr
        && g_object_get_data(G_OBJECT(static_cast<GtkWidget*>(inner)), "mpapp_suppress") != nullptr;
}

// --- GtkListBox row events ----------------------------------------------

void on_row_selected(GtkListBox* box, GtkListBoxRow* row, gpointer user_data) {
    auto* cv = static_cast<basic_collection_view*>(user_data);
    if (cv == nullptr) return;
    if (is_suppressed(box)) return;
    int idx = (row != nullptr) ? gtk_list_box_row_get_index(row) : -1;
    if (cv->selected_index.get() != idx) cv->selected_index.set(idx);
    if (idx >= 0) cv->item_tapped.emit(idx);
}

void on_selected_rows_changed(GtkListBox* box, gpointer user_data) {
    auto* cv = static_cast<basic_collection_view*>(user_data);
    if (cv == nullptr) return;
    if (is_suppressed(box)) return;
    if (cv->selection_mode.get() != collection_selection_mode::multiple) return;

    std::vector<int> idxs;
    GList* rows = gtk_list_box_get_selected_rows(box);
    for (GList* l = rows; l != nullptr; l = l->next) {
        auto* r = GTK_LIST_BOX_ROW(l->data);
        if (r != nullptr) idxs.push_back(gtk_list_box_row_get_index(r));
    }
    g_list_free(rows);
    if (cv->selected_indices.get() != idxs) {
        cv->selected_indices.set(std::move(idxs));
    }
}

// --- GtkFlowBox child events --------------------------------------------

void on_child_activated(GtkFlowBox* box, GtkFlowBoxChild* child, gpointer user_data) {
    auto* cv = static_cast<basic_collection_view*>(user_data);
    if (cv == nullptr || child == nullptr) return;
    if (is_suppressed(box)) return;
    const int idx = gtk_flow_box_child_get_index(child);
    if (cv->selected_index.get() != idx) cv->selected_index.set(idx);
    cv->item_tapped.emit(idx);
}

void on_selected_children_changed(GtkFlowBox* box, gpointer user_data) {
    auto* cv = static_cast<basic_collection_view*>(user_data);
    if (cv == nullptr) return;
    if (is_suppressed(box)) return;
    if (cv->selection_mode.get() != collection_selection_mode::multiple) return;

    std::vector<int> idxs;
    GList* children = gtk_flow_box_get_selected_children(box);
    for (GList* l = children; l != nullptr; l = l->next) {
        auto* c = GTK_FLOW_BOX_CHILD(l->data);
        if (c != nullptr) idxs.push_back(gtk_flow_box_child_get_index(c));
    }
    g_list_free(children);
    if (cv->selected_indices.get() != idxs) {
        cv->selected_indices.set(std::move(idxs));
    }
}

// --- inner-widget construction ------------------------------------------

GtkSelectionMode to_native_mode(collection_selection_mode m) {
    switch (m) {
        case collection_selection_mode::none:     return GTK_SELECTION_NONE;
        case collection_selection_mode::multiple: return GTK_SELECTION_MULTIPLE;
        case collection_selection_mode::single:
        default:                                  return GTK_SELECTION_SINGLE;
    }
}

GtkWidget* make_inner_listbox() {
    GtkWidget* w = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(w), GTK_SELECTION_SINGLE);
    return w;
}

GtkWidget* make_inner_flowbox(GtkOrientation orient) {
    GtkWidget* w = gtk_flow_box_new();
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(w), GTK_SELECTION_SINGLE);
    // HORIZONTAL: items flow LTR then wrap down (MAUI vertical_grid).
    // VERTICAL:   items flow TTB then wrap right (MAUI horizontal_grid).
    gtk_orientable_set_orientation(GTK_ORIENTABLE(w), orient);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(w), 16);
    gtk_flow_box_set_homogeneous(GTK_FLOW_BOX(w), TRUE);
    return w;
}

GtkWidget* make_inner_hbox() {
    // Single-row horizontal strip. GtkBox honors children's natural
    // size along the main axis, and the outer GtkScrolledWindow
    // provides horizontal scroll. No built-in selection — clicks on
    // children fire item_tapped via per-child gesture controllers
    // attached in append_item; selected_index tracks programmatically
    // but doesn't visually highlight (v1 trade-off).
    GtkWidget* w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_valign(w, GTK_ALIGN_CENTER);
    return w;
}

void clear_inner_listbox(GtkListBox* box) {
    GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(box));
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(box, child);
        child = next;
    }
}
void clear_inner_flowbox(GtkFlowBox* box) {
    GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(box));
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_flow_box_remove(box, child);
        child = next;
    }
}
void clear_inner_box(GtkBox* box) {
    GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(box));
    while (child != nullptr) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(box, child);
        child = next;
    }
}

// hbox tap handler — fires on press on an hbox item child. The
// per-item index lives on the widget as g_object_data("mpapp_idx").
// hbox has no native selection so no suppression check is needed.
void on_hbox_child_pressed(GtkGestureClick* gesture,
                           gint /*n_press*/, gdouble /*x*/, gdouble /*y*/,
                           gpointer user_data) {
    auto* cv = static_cast<basic_collection_view*>(user_data);
    if (cv == nullptr) return;
    GtkWidget* w = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
    if (w == nullptr) return;
    void* d = g_object_get_data(G_OBJECT(w), "mpapp_idx");
    const int idx = GPOINTER_TO_INT(d);
    if (cv->selected_index.get() != idx) cv->selected_index.set(idx);
    cv->item_tapped.emit(idx);
}

// Append a widget into an hbox-mode container, stamping it with the
// next item index and wiring a GtkGestureClick that routes taps back
// to the bound basic_collection_view.
void append_hbox_child(GtkBox* box, GtkWidget* child, int idx, basic_collection_view* cv) {
    g_object_set_data(G_OBJECT(child), "mpapp_idx", GINT_TO_POINTER(idx));
    GtkGesture* g = gtk_gesture_click_new();
    gtk_widget_add_controller(child, GTK_EVENT_CONTROLLER(g));
    g_signal_connect(g, "pressed", G_CALLBACK(on_hbox_child_pressed), cv);
    gtk_box_append(box, child);
}

} // namespace

collection_view_handler<platform::linux_>::collection_view_handler() {
    native_ = gtk_scrolled_window_new();
    inner_  = make_inner_listbox();
    kind_   = layout_kind::list;
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(static_cast<GtkWidget*>(native_)),
                                  static_cast<GtkWidget*>(inner_));
    gtk_widget_set_vexpand(static_cast<GtkWidget*>(native_), TRUE);
    gtk_widget_set_hexpand(static_cast<GtkWidget*>(native_), TRUE);
}

collection_view_handler<platform::linux_>::~collection_view_handler() = default;

void collection_view_handler<platform::linux_>::wire_tap_signals() {
    if (inner_ == nullptr || bound_ == nullptr) return;
    GtkWidget* w = static_cast<GtkWidget*>(inner_);
    switch (kind_) {
        case layout_kind::list:
            g_signal_connect(w, "row-selected",
                             G_CALLBACK(on_row_selected), bound_);
            g_signal_connect(w, "selected-rows-changed",
                             G_CALLBACK(on_selected_rows_changed), bound_);
            break;
        case layout_kind::flow_horiz:
        case layout_kind::flow_vert:
            g_signal_connect(w, "child-activated",
                             G_CALLBACK(on_child_activated), bound_);
            g_signal_connect(w, "selected-children-changed",
                             G_CALLBACK(on_selected_children_changed), bound_);
            break;
        case layout_kind::hbox:
            // hbox wires per-child gestures inside append_hbox_child
            // during rebuild — nothing to do at the container level.
            break;
        case layout_kind::unset:
            break;
    }
}

void collection_view_handler<platform::linux_>::rebuild_items(const std::vector<std::string>& v) {
    if (inner_ == nullptr) return;
    set_suppress(inner_, true);
    GtkWidget* w = static_cast<GtkWidget*>(inner_);
    switch (kind_) {
        case layout_kind::list:
            clear_inner_listbox(GTK_LIST_BOX(w));
            for (const auto& s : v) {
                GtkWidget* lbl = gtk_label_new(s.c_str());
                gtk_widget_set_halign(lbl, GTK_ALIGN_START);
                gtk_list_box_append(GTK_LIST_BOX(w), lbl);
            }
            break;
        case layout_kind::flow_horiz:
        case layout_kind::flow_vert:
            clear_inner_flowbox(GTK_FLOW_BOX(w));
            for (const auto& s : v) {
                GtkWidget* lbl = gtk_label_new(s.c_str());
                gtk_widget_set_halign(lbl, GTK_ALIGN_START);
                gtk_flow_box_append(GTK_FLOW_BOX(w), lbl);
            }
            break;
        case layout_kind::hbox: {
            clear_inner_box(GTK_BOX(w));
            int i = 0;
            for (const auto& s : v) {
                GtkWidget* lbl = gtk_label_new(s.c_str());
                gtk_widget_set_margin_start(lbl, 6);
                gtk_widget_set_margin_end(lbl, 6);
                append_hbox_child(GTK_BOX(w), lbl, i++, bound_);
            }
            break;
        }
        case layout_kind::unset:
            break;
    }
    if (bound_ != nullptr) apply_selection(bound_->selected_index.get());
    set_suppress(inner_, false);
}

void collection_view_handler<platform::linux_>::rebuild_typed(const std::vector<view*>& v) {
    if (inner_ == nullptr) return;
    set_suppress(inner_, true);
    GtkWidget* container = static_cast<GtkWidget*>(inner_);
    switch (kind_) {
        case layout_kind::list:
            clear_inner_listbox(GTK_LIST_BOX(container));
            for (view* item : v) {
                if (item == nullptr) continue;
                GtkWidget* w = detail::linux_dispatch::dispatch(item);
                if (w != nullptr) gtk_list_box_append(GTK_LIST_BOX(container), w);
            }
            break;
        case layout_kind::flow_horiz:
        case layout_kind::flow_vert:
            clear_inner_flowbox(GTK_FLOW_BOX(container));
            for (view* item : v) {
                if (item == nullptr) continue;
                GtkWidget* w = detail::linux_dispatch::dispatch(item);
                if (w != nullptr) gtk_flow_box_append(GTK_FLOW_BOX(container), w);
            }
            break;
        case layout_kind::hbox: {
            clear_inner_box(GTK_BOX(container));
            int i = 0;
            for (view* item : v) {
                if (item == nullptr) continue;
                GtkWidget* w = detail::linux_dispatch::dispatch(item);
                if (w != nullptr) append_hbox_child(GTK_BOX(container), w, i++, bound_);
            }
            break;
        }
        case layout_kind::unset:
            break;
    }
    if (bound_ != nullptr) apply_selection(bound_->selected_index.get());
    set_suppress(inner_, false);
}

void collection_view_handler<platform::linux_>::rebuild_active() {
    if (bound_ == nullptr) return;
    if (!bound_->typed_items.get().empty()) {
        rebuild_typed(bound_->typed_items.get());
    } else if (bound_->materialized_count() > 0) {
        // item_template materialized — render through typed pipeline.
        rebuild_typed(bound_->materialized_views());
    } else {
        rebuild_items(bound_->items_source.get());
    }
}

void collection_view_handler<platform::linux_>::apply_selection(int idx) {
    if (inner_ == nullptr) return;
    set_suppress(inner_, true);
    GtkWidget* w = static_cast<GtkWidget*>(inner_);
    switch (kind_) {
        case layout_kind::list: {
            GtkListBox* box = GTK_LIST_BOX(w);
            if (idx < 0) {
                gtk_list_box_unselect_all(box);
            } else {
                GtkListBoxRow* row = gtk_list_box_get_row_at_index(box, idx);
                if (row != nullptr) gtk_list_box_select_row(box, row);
            }
            break;
        }
        case layout_kind::flow_horiz:
        case layout_kind::flow_vert: {
            GtkFlowBox* box = GTK_FLOW_BOX(w);
            gtk_flow_box_unselect_all(box);
            if (idx >= 0) {
                GtkFlowBoxChild* child = gtk_flow_box_get_child_at_index(box, idx);
                if (child != nullptr) gtk_flow_box_select_child(box, child);
            }
            break;
        }
        case layout_kind::hbox:
            // GtkBox has no native selection. selected_index is tracked
            // in the C++ surface; visual highlight is a follow-up.
            break;
        case layout_kind::unset:
            break;
    }
    set_suppress(inner_, false);
}

void collection_view_handler<platform::linux_>::apply_selection_mode(collection_selection_mode m) {
    if (inner_ == nullptr) return;
    GtkWidget* w = static_cast<GtkWidget*>(inner_);
    switch (kind_) {
        case layout_kind::list:
            gtk_list_box_set_selection_mode(GTK_LIST_BOX(w), to_native_mode(m));
            break;
        case layout_kind::flow_horiz:
        case layout_kind::flow_vert:
            gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(w), to_native_mode(m));
            break;
        case layout_kind::hbox:
        case layout_kind::unset:
            break;
    }
}

void collection_view_handler<platform::linux_>::apply_layout(collection_layout l) {
    if (native_ == nullptr) return;
    layout_kind want = layout_kind::list;
    switch (l) {
        case collection_layout::vertical_list:   want = layout_kind::list;       break;
        case collection_layout::horizontal_list: want = layout_kind::hbox;       break;
        case collection_layout::vertical_grid:   want = layout_kind::flow_horiz; break;
        case collection_layout::horizontal_grid: want = layout_kind::flow_vert;  break;
    }
    if (want == kind_) return;

    // Tear down old inner; the GtkScrolledWindow drops the previous
    // child when we set a new one. Build the right inner widget +
    // adjust the scrollbar policy for the new layout's primary axis.
    GtkWidget* new_inner = nullptr;
    GtkPolicyType hpolicy = GTK_POLICY_NEVER;
    GtkPolicyType vpolicy = GTK_POLICY_AUTOMATIC;
    switch (want) {
        case layout_kind::list:
            new_inner = make_inner_listbox();
            hpolicy = GTK_POLICY_NEVER;
            vpolicy = GTK_POLICY_AUTOMATIC;
            break;
        case layout_kind::hbox:
            new_inner = make_inner_hbox();
            hpolicy = GTK_POLICY_AUTOMATIC;
            vpolicy = GTK_POLICY_NEVER;
            break;
        case layout_kind::flow_horiz:
            new_inner = make_inner_flowbox(GTK_ORIENTATION_HORIZONTAL);
            hpolicy = GTK_POLICY_NEVER;
            vpolicy = GTK_POLICY_AUTOMATIC;
            break;
        case layout_kind::flow_vert:
            new_inner = make_inner_flowbox(GTK_ORIENTATION_VERTICAL);
            hpolicy = GTK_POLICY_AUTOMATIC;
            vpolicy = GTK_POLICY_NEVER;
            break;
        case layout_kind::unset:
            return;
    }
    inner_ = new_inner;
    kind_  = want;
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(static_cast<GtkWidget*>(native_)),
                                  static_cast<GtkWidget*>(inner_));
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(static_cast<GtkWidget*>(native_)),
                                   hpolicy, vpolicy);
    wire_tap_signals();

    if (bound_ != nullptr) {
        apply_selection_mode(bound_->selection_mode.get());
        rebuild_active();
    }
}

void collection_view_handler<platform::linux_>::map_items_source(basic_collection_view& cv) {
    bound_ = &cv;
    wire_tap_signals();
    rebuild_active();
    cv.items_source.changed.subscribe(items_slot_, items_cb_);
}

void collection_view_handler<platform::linux_>::map_typed_items(basic_collection_view& cv) {
    bound_ = &cv;
    wire_tap_signals();
    rebuild_active();
    cv.typed_items.changed.subscribe(typed_slot_, typed_cb_);
    cv.materialized_changed.subscribe(materialized_slot_, materialized_cb_);
}

void collection_view_handler<platform::linux_>::map_selected_index(basic_collection_view& cv) {
    apply_selection(cv.selected_index.get());
    cv.selected_index.changed.subscribe(sel_slot_, sel_cb_);
}

void collection_view_handler<platform::linux_>::map_selection_mode(basic_collection_view& cv) {
    apply_selection_mode(cv.selection_mode.get());
    cv.selection_mode.changed.subscribe(mode_slot_, mode_cb_);
}

void collection_view_handler<platform::linux_>::map_layout(basic_collection_view& cv) {
    apply_layout(cv.layout.get());
    cv.layout.changed.subscribe(layout_slot_, layout_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_collection_view(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::internal::basic_collection_view*>(v); c && c->has_cv_handler()) {
        return GTK_WIDGET(c->cv_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_collection_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
