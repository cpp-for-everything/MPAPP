// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 follow-up — GTK4 entry handler implementation.

#include "mpapp/handlers/linux/entry_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp {

namespace {

struct on_changed_ctx {
    entry*                            target;
    entry_handler<platform::linux_>*  handler;
};

void on_entry_changed(GtkEditable* editable, gpointer user_data) {
    auto* ctx = static_cast<on_changed_ctx*>(user_data);
    if (ctx == nullptr || ctx->target == nullptr) return;
    // The context is stable for the handler's lifetime; its `handler`
    // pointer is used as a flag-only reference (suppression).
    // We rely on the apply_text() suppression flag set on the handler.
    const char* text = gtk_editable_get_text(editable);
    if (text == nullptr) text = "";
    const std::string s{text};
    if (ctx->target->text.get() != s) {
        ctx->target->text.set(s);
    }
}

} // namespace

entry_handler<platform::linux_>::entry_handler() {
    native_ = gtk_entry_new();
}

entry_handler<platform::linux_>::~entry_handler() {
    if (native_ != nullptr && changed_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    changed_handler_id_);
        changed_handler_id_ = 0;
    }
}

void entry_handler<platform::linux_>::apply_text(const std::string& text) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    gtk_editable_set_text(GTK_EDITABLE(static_cast<GtkWidget*>(native_)),
                          text.c_str());
    suppress_echo_ = false;
}

void entry_handler<platform::linux_>::apply_placeholder(const std::string& text) {
    if (native_ != nullptr) {
        gtk_entry_set_placeholder_text(
            GTK_ENTRY(static_cast<GtkWidget*>(native_)), text.c_str());
    }
}

void entry_handler<platform::linux_>::apply_is_read_only(bool ro) {
    if (native_ != nullptr) {
        gtk_editable_set_editable(
            GTK_EDITABLE(static_cast<GtkWidget*>(native_)), !ro);
    }
}

void entry_handler<platform::linux_>::map_text(entry& e) {
    apply_text(e.text.get());
    e.text.changed.subscribe(text_slot_, text_cb_);

    if (native_ == nullptr) return;
    // Leaked ctx is fine — its lifetime is the handler's lifetime, and
    // the handler is non-movable. The signal handler ID gets
    // disconnected in the destructor.
    auto* ctx = new on_changed_ctx{&e, this};
    if (changed_handler_id_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    changed_handler_id_);
    }
    changed_handler_id_ = g_signal_connect_data(
        static_cast<GtkWidget*>(native_),
        "changed",
        G_CALLBACK(on_entry_changed),
        ctx,
        +[](gpointer p, GClosure*) { delete static_cast<on_changed_ctx*>(p); },
        static_cast<GConnectFlags>(0));
}

void entry_handler<platform::linux_>::map_placeholder(entry& e) {
    apply_placeholder(e.placeholder.get());
    e.placeholder.changed.subscribe(placeholder_slot_, placeholder_cb_);
}

void entry_handler<platform::linux_>::map_is_read_only(entry& e) {
    apply_is_read_only(e.is_read_only.get());
    e.is_read_only.changed.subscribe(readonly_slot_, readonly_cb_);
}

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
