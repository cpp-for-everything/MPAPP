// SPDX-License-Identifier: Apache-2.0
// GTK4 hybrid_web_view handler implementation.

#include "mpapp/handlers/linux/hybrid_web_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__) && defined(MPAPP_HAS_WEBKITGTK)

#include <gtk/gtk.h>
#include <webkit/webkit.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp {

namespace {

constexpr const char* kBridgeShim =
    "(function(){"
    "  if (window.mpapp && window.mpapp.__mpapp) return;"
    "  var listeners = [];"
    "  window.mpapp = {"
    "    __mpapp: true,"
    "    send: function(p) { window.webkit.messageHandlers.mpapp_send.postMessage(String(p)); },"
    "    on:   function(fn) { listeners.push(fn); },"
    "    _receive: function(p) { for (var i=0; i<listeners.length; ++i) try { listeners[i](p); } catch(e) {} }"
    "  };"
    "})();";

struct script_msg_ctx {
    hybrid_web_view_handler<platform::linux_>* handler;
};

// WebKitGTK 6.x emits "script-message-received" with a JSCValue* —
// the JavaScript-side argument arrives as a typed JSC value.
void on_script_message(WebKitUserContentManager* /*mgr*/,
                       JSCValue* value,
                       gpointer user_data) {
    auto* ctx = static_cast<script_msg_ctx*>(user_data);
    if (ctx == nullptr || ctx->handler == nullptr || value == nullptr) return;
    if (!jsc_value_is_string(value)) return;
    gchar* utf8 = jsc_value_to_string(value);
    if (utf8 == nullptr) return;
    ctx->handler->on_native_inbound(std::string{utf8});
    g_free(utf8);
}

} // namespace

hybrid_web_view_handler<platform::linux_>::hybrid_web_view_handler() {
    // WebKitGTK 6.x: every WebKitWebView is constructed with an internal
    // default user-content-manager. Reuse that rather than the older
    // 4.1 `webkit_web_view_new_with_user_content_manager` helper.
    native_      = webkit_web_view_new();
    content_mgr_ = webkit_web_view_get_user_content_manager(
        WEBKIT_WEB_VIEW(static_cast<GtkWidget*>(native_)));

    // Inject the JS shim at document-start.
    if (content_mgr_ != nullptr) {
        WebKitUserScript* script = webkit_user_script_new(
            kBridgeShim,
            WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
            WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
            nullptr,
            nullptr);
        webkit_user_content_manager_add_script(
            WEBKIT_USER_CONTENT_MANAGER(static_cast<GObject*>(content_mgr_)),
            script);
        webkit_user_script_unref(script);
    }
}

hybrid_web_view_handler<platform::linux_>::~hybrid_web_view_handler() {
    if (content_mgr_ != nullptr && msg_handler_id_ != 0) {
        g_signal_handler_disconnect(
            G_OBJECT(static_cast<GObject*>(content_mgr_)),
            msg_handler_id_);
        msg_handler_id_ = 0;
    }
}

void hybrid_web_view_handler<platform::linux_>::send_outbound(const std::string& payload) {
    if (native_ == nullptr) return;
    // Escape the payload as a JSON string literal so embedded quotes /
    // newlines don't break the eval.
    std::string esc;
    esc.reserve(payload.size() + 16);
    esc.push_back('"');
    for (char c : payload) {
        switch (c) {
            case '\\': esc += "\\\\"; break;
            case '"':  esc += "\\\""; break;
            case '\n': esc += "\\n";  break;
            case '\r': esc += "\\r";  break;
            case '\t': esc += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(static_cast<unsigned char>(c)));
                    esc += buf;
                } else {
                    esc.push_back(c);
                }
        }
    }
    esc.push_back('"');
    std::string js = std::string{"window.mpapp && window.mpapp._receive("} + esc + std::string{");"};

    // WebKitGTK 6.x evaluate_javascript signature:
    // (web_view, body, length, world_name, source_uri, cancellable, callback, user_data)
    webkit_web_view_evaluate_javascript(
        WEBKIT_WEB_VIEW(static_cast<GtkWidget*>(native_)),
        js.c_str(), -1, nullptr, nullptr, nullptr, nullptr, nullptr);
}

void hybrid_web_view_handler<platform::linux_>::on_native_inbound(const std::string& payload) {
    if (bound_ == nullptr) return;
    // Single choke point — hybrid_web_view::process_inbound decides
    // whether to route through an attached bridge or fall through to
    // the raw message_received signal.
    bound_->process_inbound(payload);
}

void hybrid_web_view_handler<platform::linux_>::map_messages(hybrid_web_view& h) {
    bound_ = &h;
    h.message_sent.subscribe(sent_slot_, sent_cb_);

    if (content_mgr_ == nullptr || wired_) return;

    auto* mgr = WEBKIT_USER_CONTENT_MANAGER(static_cast<GObject*>(content_mgr_));
    // WebKitGTK 6.x signature: (mgr, name, world_name).
    webkit_user_content_manager_register_script_message_handler(mgr, "mpapp_send", nullptr);

    auto* ctx = new script_msg_ctx{this};
    msg_handler_id_ = g_signal_connect_data(
        G_OBJECT(mgr),
        "script-message-received::mpapp_send",
        G_CALLBACK(on_script_message),
        ctx,
        +[](gpointer p, GClosure*) { delete static_cast<script_msg_ctx*>(p); },
        static_cast<GConnectFlags>(0));
    wired_ = true;
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_hybrid_web_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::hybrid_web_view*>(v); w && w->has_hwv_handler()) {
        return GTK_WIDGET(w->hwv_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_hybrid_web_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#elif defined(__linux__) && !defined(__ANDROID__)

// Stub when WebKitGTK isn't found at configure time.
#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp {

hybrid_web_view_handler<platform::linux_>::hybrid_web_view_handler() = default;
hybrid_web_view_handler<platform::linux_>::~hybrid_web_view_handler() = default;

void hybrid_web_view_handler<platform::linux_>::send_outbound(const std::string&) {}
void hybrid_web_view_handler<platform::linux_>::on_native_inbound(const std::string&) {}

void hybrid_web_view_handler<platform::linux_>::map_messages(hybrid_web_view& h) {
    bound_ = &h;
    h.message_sent.subscribe(sent_slot_, sent_cb_);
}

} // namespace mpapp

namespace {
GtkWidget* dispatch_hybrid_web_view_stub(::mpapp::view*) { return nullptr; }
struct registrar_stub {
    registrar_stub() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_hybrid_web_view_stub); }
};
[[maybe_unused]] registrar_stub _reg_stub;
} // namespace

#endif
