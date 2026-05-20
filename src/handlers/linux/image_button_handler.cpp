// SPDX-License-Identifier: Apache-2.0
// GTK4 image_button handler implementation.

#include "mpapp/handlers/linux/image_button_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp {

image_button_handler<platform::linux_>::image_button_handler() {
    GtkWidget* btn = gtk_button_new();
    GtkWidget* pic = gtk_picture_new();
    gtk_button_set_child(GTK_BUTTON(btn), pic);
    native_  = btn;
    picture_ = pic;
}

image_button_handler<platform::linux_>::~image_button_handler() = default;

void image_button_handler<platform::linux_>::apply_source(const std::string& v) {
    if (picture_ == nullptr) return;
    GtkPicture* pic = GTK_PICTURE(static_cast<GtkWidget*>(picture_));
    if (v.empty()) { gtk_picture_set_paintable(pic, nullptr); return; }
    gtk_picture_set_filename(pic, v.c_str());
}

void image_button_handler<platform::linux_>::apply_aspect(aspect_mode v) {
    if (picture_ == nullptr) return;
    GtkPicture* pic = GTK_PICTURE(static_cast<GtkWidget*>(picture_));
    switch (v) {
        case aspect_mode::aspect_fit:  gtk_picture_set_content_fit(pic, GTK_CONTENT_FIT_CONTAIN);    break;
        case aspect_mode::aspect_fill: gtk_picture_set_content_fit(pic, GTK_CONTENT_FIT_COVER);      break;
        case aspect_mode::fill:        gtk_picture_set_content_fit(pic, GTK_CONTENT_FIT_FILL);       break;
        case aspect_mode::center:      gtk_picture_set_content_fit(pic, GTK_CONTENT_FIT_SCALE_DOWN); break;
    }
}

void image_button_handler<platform::linux_>::map_source(image_button& b) {
    apply_source(b.source.get());
    b.source.changed.subscribe(source_slot_, source_cb_);
}
void image_button_handler<platform::linux_>::map_aspect(image_button& b) {
    apply_aspect(b.aspect.get());
    b.aspect.changed.subscribe(aspect_slot_, aspect_cb_);
}

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
