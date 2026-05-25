// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_title_bar handler implementation.

#include "mpapp/handlers/linux/title_bar_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

// Build the title-widget box: a vertical GtkBox containing the title
// GtkLabel and the subtitle GtkLabel. GTK4 removed the per-headerbar
// subtitle slot that GTK3 had, so we synthesise it.
GtkWidget* build_title_box(GtkWidget** out_title, GtkWidget** out_subtitle) {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget* title    = gtk_label_new("");
    GtkWidget* subtitle = gtk_label_new("");

    // Match GNOME's default headerbar styling.
    gtk_widget_add_css_class(title,    "title");
    gtk_widget_add_css_class(subtitle, "subtitle");

    gtk_label_set_single_line_mode(GTK_LABEL(title),    TRUE);
    gtk_label_set_single_line_mode(GTK_LABEL(subtitle), TRUE);
    gtk_label_set_ellipsize(GTK_LABEL(title),    PANGO_ELLIPSIZE_END);
    gtk_label_set_ellipsize(GTK_LABEL(subtitle), PANGO_ELLIPSIZE_END);

    gtk_box_append(GTK_BOX(box), title);
    gtk_box_append(GTK_BOX(box), subtitle);

    // Subtitle hides itself when empty.
    gtk_widget_set_visible(subtitle, FALSE);

    *out_title    = title;
    *out_subtitle = subtitle;
    return box;
}

} // namespace

title_bar_handler<platform::linux_>::title_bar_handler() {
    GtkWidget* header = gtk_header_bar_new();

    GtkWidget* title_label    = nullptr;
    GtkWidget* subtitle_label = nullptr;
    GtkWidget* box            = build_title_box(&title_label, &subtitle_label);

    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header), box);

    native_         = header;
    title_label_    = title_label;
    subtitle_label_ = subtitle_label;
}

title_bar_handler<platform::linux_>::~title_bar_handler() = default;

void title_bar_handler<platform::linux_>::apply_title(const std::string& v) {
    if (title_label_ == nullptr) return;
    gtk_label_set_text(GTK_LABEL(static_cast<GtkWidget*>(title_label_)), v.c_str());
}

void title_bar_handler<platform::linux_>::apply_subtitle(const std::string& v) {
    if (subtitle_label_ == nullptr) return;
    GtkWidget* w = static_cast<GtkWidget*>(subtitle_label_);
    gtk_label_set_text(GTK_LABEL(w), v.c_str());
    gtk_widget_set_visible(w, !v.empty());
}

void title_bar_handler<platform::linux_>::map_title(basic_title_bar& t) {
    apply_title(t.title.get());
    t.title.changed.subscribe(title_slot_, title_cb_);
}

void title_bar_handler<platform::linux_>::map_subtitle(basic_title_bar& t) {
    apply_subtitle(t.subtitle.get());
    t.subtitle.changed.subscribe(subtitle_slot_, subtitle_cb_);
}

void title_bar_handler<platform::linux_>::map_gestures(basic_title_bar& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// --- ADR-0013 self-registration --------------------------------------------

namespace {

GtkWidget* dispatch_title_bar(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_title_bar*>(v); w && w->has_handler()) {
        return static_cast<GtkWidget*>(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_title_bar);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
