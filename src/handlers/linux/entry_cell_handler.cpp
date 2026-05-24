// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_entry_cell handler implementation.

#include "mpapp/handlers/linux/entry_cell_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

struct entry_ctx {
    basic_entry_cell*                            target;
    entry_cell_handler<platform::linux_>*  handler;
};

void on_changed(GtkEditable* editable, gpointer user_data) {
    auto* ctx = static_cast<entry_ctx*>(user_data);
    if (ctx == nullptr || ctx->target == nullptr) return;
    const char* text = gtk_editable_get_text(editable);
    if (text == nullptr) text = "";
    const std::string s{text};
    if (ctx->target->text.get() != s) {
        ctx->target->text.set(s);
    }
}

void on_activate(GtkEntry* /*basic_entry*/, gpointer user_data) {
    auto* ctx = static_cast<entry_ctx*>(user_data);
    if (ctx == nullptr || ctx->target == nullptr) return;
    ctx->target->completed.emit(ctx->target->text.get());
}

GtkInputPurpose keyboard_to_purpose(keyboard_kind k) {
    switch (k) {
        case keyboard_kind::email:     return GTK_INPUT_PURPOSE_EMAIL;
        case keyboard_kind::numeric:   return GTK_INPUT_PURPOSE_DIGITS;
        case keyboard_kind::telephone: return GTK_INPUT_PURPOSE_PHONE;
        case keyboard_kind::url:       return GTK_INPUT_PURPOSE_URL;
        case keyboard_kind::chat:      return GTK_INPUT_PURPOSE_FREE_FORM;
        case keyboard_kind::text:
        case keyboard_kind::default_:
        default:                       return GTK_INPUT_PURPOSE_FREE_FORM;
    }
}

} // namespace

entry_cell_handler<platform::linux_>::entry_cell_handler() {
    native_  = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    label_w_ = gtk_label_new("");
    entry_w_ = gtk_entry_new();

    gtk_widget_set_halign(static_cast<GtkWidget*>(label_w_), GTK_ALIGN_START);
    gtk_widget_set_valign(static_cast<GtkWidget*>(label_w_), GTK_ALIGN_CENTER);

    gtk_widget_set_hexpand(static_cast<GtkWidget*>(entry_w_), TRUE);
    gtk_widget_set_valign (static_cast<GtkWidget*>(entry_w_), GTK_ALIGN_CENTER);

    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(native_)),
                   static_cast<GtkWidget*>(label_w_));
    gtk_box_append(GTK_BOX(static_cast<GtkWidget*>(native_)),
                   static_cast<GtkWidget*>(entry_w_));

    gtk_widget_set_margin_start (static_cast<GtkWidget*>(native_), 12);
    gtk_widget_set_margin_end   (static_cast<GtkWidget*>(native_), 12);
    gtk_widget_set_margin_top   (static_cast<GtkWidget*>(native_), 6);
    gtk_widget_set_margin_bottom(static_cast<GtkWidget*>(native_), 6);
}

entry_cell_handler<platform::linux_>::~entry_cell_handler() {
    if (entry_w_ != nullptr) {
        if (changed_handler_id_ != 0) {
            g_signal_handler_disconnect(static_cast<GtkWidget*>(entry_w_),
                                        changed_handler_id_);
            changed_handler_id_ = 0;
        }
        if (activate_handler_id_ != 0) {
            g_signal_handler_disconnect(static_cast<GtkWidget*>(entry_w_),
                                        activate_handler_id_);
            activate_handler_id_ = 0;
        }
    }
}

void entry_cell_handler<platform::linux_>::apply_label(const std::string& v) {
    if (label_w_ == nullptr) return;
    gtk_label_set_text(GTK_LABEL(static_cast<GtkWidget*>(label_w_)), v.c_str());
}

void entry_cell_handler<platform::linux_>::apply_text(const std::string& v) {
    if (entry_w_ == nullptr) return;
    suppress_echo_ = true;
    gtk_editable_set_text(GTK_EDITABLE(static_cast<GtkWidget*>(entry_w_)), v.c_str());
    suppress_echo_ = false;
}

void entry_cell_handler<platform::linux_>::apply_placeholder(const std::string& v) {
    if (entry_w_ == nullptr) return;
    gtk_entry_set_placeholder_text(GTK_ENTRY(static_cast<GtkWidget*>(entry_w_)), v.c_str());
}

void entry_cell_handler<platform::linux_>::apply_keyboard(keyboard_kind k) {
    if (entry_w_ == nullptr) return;
    gtk_entry_set_input_purpose(GTK_ENTRY(static_cast<GtkWidget*>(entry_w_)),
                                keyboard_to_purpose(k));
}

void entry_cell_handler<platform::linux_>::map_label(basic_entry_cell& c) {
    apply_label(c.label.get());
    c.label.changed.subscribe(label_slot_, label_cb_);
}

void entry_cell_handler<platform::linux_>::map_text(basic_entry_cell& c) {
    bound_ = &c;
    apply_text(c.text.get());
    c.text.changed.subscribe(text_slot_, text_cb_);

    if (entry_w_ == nullptr) return;
    if (changed_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(entry_w_),
                                    changed_handler_id_);
        changed_handler_id_ = 0;
    }
    if (activate_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(entry_w_),
                                    activate_handler_id_);
        activate_handler_id_ = 0;
    }
    auto* ctx = new entry_ctx{&c, this};
    changed_handler_id_ = g_signal_connect_data(
        static_cast<GtkWidget*>(entry_w_),
        "changed",
        G_CALLBACK(on_changed),
        ctx,
        +[](gpointer p, GClosure*) { delete static_cast<entry_ctx*>(p); },
        static_cast<GConnectFlags>(0));
    // Second ctx for activate (delete is one-shot per connect).
    auto* ctx2 = new entry_ctx{&c, this};
    activate_handler_id_ = g_signal_connect_data(
        static_cast<GtkWidget*>(entry_w_),
        "activate",
        G_CALLBACK(on_activate),
        ctx2,
        +[](gpointer p, GClosure*) { delete static_cast<entry_ctx*>(p); },
        static_cast<GConnectFlags>(0));
}

void entry_cell_handler<platform::linux_>::map_placeholder(basic_entry_cell& c) {
    apply_placeholder(c.placeholder.get());
    c.placeholder.changed.subscribe(placeholder_slot_, placeholder_cb_);
}

void entry_cell_handler<platform::linux_>::map_keyboard(basic_entry_cell& c) {
    apply_keyboard(c.keyboard.get());
    c.keyboard.changed.subscribe(keyboard_slot_, keyboard_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_entry_cell(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::internal::basic_entry_cell*>(v); c && c->has_ec_handler()) {
        return GTK_WIDGET(c->ec_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_entry_cell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
