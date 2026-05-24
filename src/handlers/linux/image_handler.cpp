// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_image handler implementation.

#include "mpapp/handlers/linux/image_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp::internal {

image_handler<platform::linux_>::image_handler() {
    native_ = gtk_picture_new();
}

image_handler<platform::linux_>::~image_handler() = default;

void image_handler<platform::linux_>::apply_source(const std::string& v) {
    if (native_ == nullptr) return;
    GtkPicture* pic = GTK_PICTURE(static_cast<GtkWidget*>(native_));
    if (v.empty()) {
        gtk_picture_set_paintable(pic, nullptr);
        return;
    }
    gtk_picture_set_filename(pic, v.c_str());
}

void image_handler<platform::linux_>::apply_aspect(aspect_mode v) {
    if (native_ == nullptr) return;
    GtkPicture* pic = GTK_PICTURE(static_cast<GtkWidget*>(native_));
    switch (v) {
        case aspect_mode::aspect_fit:
            gtk_picture_set_content_fit(pic, GTK_CONTENT_FIT_CONTAIN);
            break;
        case aspect_mode::aspect_fill:
            gtk_picture_set_content_fit(pic, GTK_CONTENT_FIT_COVER);
            break;
        case aspect_mode::fill:
            gtk_picture_set_content_fit(pic, GTK_CONTENT_FIT_FILL);
            break;
        case aspect_mode::center:
            gtk_picture_set_content_fit(pic, GTK_CONTENT_FIT_SCALE_DOWN);
            break;
    }
}

void image_handler<platform::linux_>::map_source(basic_image& i) {
    apply_source(i.source.get());
    i.source.changed.subscribe(source_slot_, source_cb_);
}
void image_handler<platform::linux_>::map_aspect(basic_image& i) {
    apply_aspect(i.aspect.get());
    i.aspect.changed.subscribe(aspect_slot_, aspect_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_image so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/internal/basic_image.hpp"

namespace {

GtkWidget* dispatch_image(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_image*>(v); w && w->has_handler()) {
        return GTK_WIDGET(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_image); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
