// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_editor handler implementation.

#include "mpapp/handlers/linux/editor_handler.hpp"

#include "mpapp/handlers/linux/gesture_attach.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <gtk/gtk.h>

namespace mpapp::internal {

namespace {

struct changed_ctx {
    basic_editor*                            target;
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

void editor_handler<platform::linux_>::map_text(basic_editor& e) {
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

void editor_handler<platform::linux_>::map_is_read_only(basic_editor& e) {
    apply_is_read_only(e.is_read_only.get());
    e.is_read_only.changed.subscribe(readonly_slot_, readonly_cb_);
}

void editor_handler<platform::linux_>::map_gestures(basic_editor& x) {
    if (native_ == nullptr) return;
    linux_gestures::attach(static_cast<GtkWidget*>(native_), x);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_editor so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/linux/widget_dispatch.hpp"
#include "mpapp/internal/basic_editor.hpp"

namespace {

GtkWidget* dispatch_editor(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_editor*>(v); w && w->has_handler()) {
        return GTK_WIDGET(w->handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_editor); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __linux__ && !__ANDROID__
