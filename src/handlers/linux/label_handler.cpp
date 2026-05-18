// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — GTK4 label handler implementation.

#include "mpapp/handlers/linux/label_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp {

label_handler<platform::linux_>::label_handler() {
    native_ = gtk_label_new("");
}

label_handler<platform::linux_>::~label_handler() = default;

void label_handler<platform::linux_>::apply_text(const std::string& text) {
    if (native_ != nullptr) {
        gtk_label_set_text(GTK_LABEL(static_cast<GtkWidget*>(native_)),
                           text.c_str());
    }
}

void label_handler<platform::linux_>::map_text(label& l) {
    apply_text(l.text.get());
    l.text.changed.subscribe(text_slot_, text_cb_);
}

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
