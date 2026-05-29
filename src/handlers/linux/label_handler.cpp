// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — GTK4 basic_label handler implementation.

#include "mpapp/handlers/linux/label_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp::internal {

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

void label_handler<platform::linux_>::map_text(basic_label& l) {
    apply_text(l.text.get());
    l.text.changed.subscribe(text_slot_, text_cb_);
}

void label_handler<platform::linux_>::apply_font() {
    if (native_ == nullptr) return;
    PangoAttrList* attrs = pango_attr_list_new();
    if (font_size_ > 0.0) {
        pango_attr_list_insert(
            attrs, pango_attr_size_new(static_cast<int>(font_size_ * PANGO_SCALE)));
    }
    if (font_bold_) {
        pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    }
    if (!font_family_.empty()) {
        pango_attr_list_insert(attrs, pango_attr_family_new(font_family_.c_str()));
    }
    if (text_color_.a > 0.0) {
        auto to16 = [](double c) {
            if (c < 0.0) c = 0.0;
            if (c > 1.0) c = 1.0;
            return static_cast<guint16>(c * 65535.0);
        };
        pango_attr_list_insert(attrs, pango_attr_foreground_new(
            to16(text_color_.r), to16(text_color_.g), to16(text_color_.b)));
    }
    gtk_label_set_attributes(GTK_LABEL(static_cast<GtkWidget*>(native_)), attrs);
    pango_attr_list_unref(attrs);
}

void label_handler<platform::linux_>::map_text_color(basic_label& l) {
    text_color_ = l.text_color.get();
    apply_font();
    l.text_color.changed.subscribe(tcolor_slot_, tcolor_cb_);
}

void label_handler<platform::linux_>::map_font_size(basic_label& l) {
    font_size_ = l.font_size.get();
    apply_font();
    l.font_size.changed.subscribe(fsize_slot_, fsize_cb_);
}

void label_handler<platform::linux_>::map_font_bold(basic_label& l) {
    font_bold_ = l.font_bold.get();
    apply_font();
    l.font_bold.changed.subscribe(fbold_slot_, fbold_cb_);
}

void label_handler<platform::linux_>::map_font_family(basic_label& l) {
    font_family_ = l.font_family.get();
    apply_font();
    l.font_family.changed.subscribe(ffamily_slot_, ffamily_cb_);
}

void label_handler<platform::linux_>::map_gestures(basic_label& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_label so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/internal/basic_label.hpp"

namespace {

GtkWidget* dispatch_label(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_label*>(v); w && w->has_handler()) {
        return GTK_WIDGET(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_label); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
