// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 editor handler implementation.

#include "mpapp/handlers/linux/editor_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp {

namespace {

struct changed_ctx {
    editor*                            target;
    editor_handler<platform::linux_>*  handler;
};

void on_buffer_changed(GtkTextBuffer* buf, gpointer user_data) {
    auto* ctx = static_cast<changed_ctx*>(user_data);
    if (ctx == nullptr || ctx->target == nullptr) return;
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buf, &start, &end);
    char* utf8 = gtk_text_buffer_get_text(buf, &start, &end, FALSE);
    const std::string s = utf8 != nullptr ? std::string{utf8} : std::string{};
    if (utf8 != nullptr) g_free(utf8);
    if (ctx->target->text.get() != s) {
        ctx->target->text.set(s);
    }
}

GtkTextBuffer* get_buffer(void* native) {
    return gtk_text_view_get_buffer(GTK_TEXT_VIEW(static_cast<GtkWidget*>(native)));
}

} // namespace

editor_handler<platform::linux_>::editor_handler() {
    native_ = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(static_cast<GtkWidget*>(native_)),
                                GTK_WRAP_WORD_CHAR);
    gtk_widget_set_size_request(static_cast<GtkWidget*>(native_), -1, 80);
}

editor_handler<platform::linux_>::~editor_handler() {
    if (native_ != nullptr && changed_handler_id_ != 0) {
        g_signal_handler_disconnect(G_OBJECT(get_buffer(native_)),
                                    changed_handler_id_);
        changed_handler_id_ = 0;
    }
}

void editor_handler<platform::linux_>::apply_text(const std::string& text) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    gtk_text_buffer_set_text(get_buffer(native_),
                             text.c_str(),
                             static_cast<int>(text.size()));
    suppress_echo_ = false;
}

void editor_handler<platform::linux_>::apply_is_read_only(bool ro) {
    if (native_ == nullptr) return;
    gtk_text_view_set_editable(GTK_TEXT_VIEW(static_cast<GtkWidget*>(native_)),
                               ro ? FALSE : TRUE);
}

void editor_handler<platform::linux_>::map_text(editor& e) {
    apply_text(e.text.get());
    e.text.changed.subscribe(text_slot_, text_cb_);

    if (native_ == nullptr) return;
    GtkTextBuffer* buf = get_buffer(native_);
    auto* ctx = new changed_ctx{&e, this};
    if (changed_handler_id_ != 0) {
        g_signal_handler_disconnect(G_OBJECT(buf), changed_handler_id_);
    }
    changed_handler_id_ = g_signal_connect_data(
        buf, "changed",
        G_CALLBACK(on_buffer_changed),
        ctx,
        +[](gpointer p, GClosure*) { delete static_cast<changed_ctx*>(p); },
        static_cast<GConnectFlags>(0));
}

void editor_handler<platform::linux_>::map_is_read_only(editor& e) {
    apply_is_read_only(e.is_read_only.get());
    e.is_read_only.changed.subscribe(readonly_slot_, readonly_cb_);
}

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
