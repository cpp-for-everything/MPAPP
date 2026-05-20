// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 time_picker handler implementation.

#include "mpapp/handlers/linux/time_picker_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp {

time_picker_handler<platform::linux_>::time_picker_handler() {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    native_ = box;

    hour_   = gtk_spin_button_new_with_range(0, 23, 1);
    minute_ = gtk_spin_button_new_with_range(0, 59, 1);

    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(static_cast<GtkWidget*>(hour_)),   0);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(static_cast<GtkWidget*>(minute_)), 0);

    gtk_box_append(GTK_BOX(box), GTK_WIDGET(static_cast<GtkWidget*>(hour_)));
    GtkWidget* sep = gtk_label_new(":");
    gtk_box_append(GTK_BOX(box), sep);
    gtk_box_append(GTK_BOX(box), GTK_WIDGET(static_cast<GtkWidget*>(minute_)));
}

time_picker_handler<platform::linux_>::~time_picker_handler() = default;

void time_picker_handler<platform::linux_>::apply_time(const time_value& v) {
    if (hour_ != nullptr) {
        gtk_spin_button_set_value(
            GTK_SPIN_BUTTON(static_cast<GtkWidget*>(hour_)), v.hour);
    }
    if (minute_ != nullptr) {
        gtk_spin_button_set_value(
            GTK_SPIN_BUTTON(static_cast<GtkWidget*>(minute_)), v.minute);
    }
}

void time_picker_handler<platform::linux_>::map_time(time_picker& p) {
    apply_time(p.time.get());
    p.time.changed.subscribe(time_slot_, time_cb_);
}

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
