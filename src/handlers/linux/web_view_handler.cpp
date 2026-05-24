// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_web_view handler implementation. WebKitGTK 6.x — dynamically
// linked, LGPL runtime per Rule 9 / RFC-0001.

#include "mpapp/handlers/linux/web_view_handler.hpp"

#if defined(__linux__) && !defined(__ANDROID__) && defined(MPAPP_HAS_WEBKITGTK)

#include <gtk/gtk.h>
#include <webkit/webkit.h>

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

struct load_ctx {
    basic_web_view*                          target;
    web_view_handler<platform::linux_>* handler;
};

void on_load_changed(WebKitWebView* wv, WebKitLoadEvent evt, gpointer user_data) {
    auto* ctx = static_cast<load_ctx*>(user_data);
    if (ctx == nullptr || ctx->target == nullptr) return;

    auto current_uri = [&]() -> std::string {
        const gchar* u = webkit_web_view_get_uri(wv);
        return u ? std::string{u} : std::string{};
    };

    switch (evt) {
        case WEBKIT_LOAD_STARTED: {
            ctx->target->is_loading.set(true);
            ctx->target->navigating.emit(current_uri());
            break;
        }
        case WEBKIT_LOAD_REDIRECTED:
        case WEBKIT_LOAD_COMMITTED:
            // intermediate stages — leave is_loading true
            break;
        case WEBKIT_LOAD_FINISHED: {
            ctx->target->is_loading.set(false);
            ctx->target->can_go_back.set(webkit_web_view_can_go_back(wv) == TRUE);
            ctx->target->can_go_forward.set(webkit_web_view_can_go_forward(wv) == TRUE);
            ctx->target->navigated.emit(current_uri(), true);
            break;
        }
        default:
            break;
    }
}

} // namespace

web_view_handler<platform::linux_>::web_view_handler() {
    native_ = webkit_web_view_new();
}

web_view_handler<platform::linux_>::~web_view_handler() {
    if (native_ != nullptr && load_changed_handler_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    load_changed_handler_);
        load_changed_handler_ = 0;
    }
}

void web_view_handler<platform::linux_>::apply_url(const std::string& v) {
    if (native_ == nullptr) return;
    if (v.empty()) return;
    suppress_echo_ = true;
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(static_cast<GtkWidget*>(native_)),
                             v.c_str());
    suppress_echo_ = false;
}

void web_view_handler<platform::linux_>::apply_html(const std::string& v) {
    if (native_ == nullptr) return;
    if (v.empty()) return;
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(static_cast<GtkWidget*>(native_)),
                              v.c_str(),
                              nullptr /* base_uri */);
}

void web_view_handler<platform::linux_>::map_url(basic_web_view& wv) {
    bound_ = &wv;
    apply_url(wv.url.get());
    wv.url.changed.subscribe(url_slot_, url_cb_);

    if (native_ == nullptr) return;
    if (load_changed_handler_ != 0) {
        g_signal_handler_disconnect(static_cast<GtkWidget*>(native_),
                                    load_changed_handler_);
        load_changed_handler_ = 0;
    }
    auto* ctx = new load_ctx{&wv, this};
    load_changed_handler_ = g_signal_connect_data(
        static_cast<GtkWidget*>(native_),
        "load-changed",
        G_CALLBACK(on_load_changed),
        ctx,
        +[](gpointer p, GClosure*) { delete static_cast<load_ctx*>(p); },
        static_cast<GConnectFlags>(0));
}

void web_view_handler<platform::linux_>::map_html(basic_web_view& wv) {
    apply_html(wv.html_source.get());
    wv.html_source.changed.subscribe(html_slot_, html_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

GtkWidget* dispatch_web_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_web_view*>(v); w && w->has_wv_handler()) {
        return GTK_WIDGET(w->wv_handler().native());
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_web_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#elif defined(__linux__) && !defined(__ANDROID__)

// Stub when WebKitGTK isn't found at configure time. Constructors / map
// methods are valid but the widget cannot render content. Keeps the
// build link-clean so an example using basic_web_view still compiles even on
// hosts without webkitgtk installed.

#include "mpapp/handlers/linux/widget_dispatch.hpp"

namespace mpapp::internal {

web_view_handler<platform::linux_>::web_view_handler() = default;
web_view_handler<platform::linux_>::~web_view_handler() = default;

void web_view_handler<platform::linux_>::apply_url(const std::string&)  {}
void web_view_handler<platform::linux_>::apply_html(const std::string&) {}

void web_view_handler<platform::linux_>::map_url(basic_web_view& wv) {
    bound_ = &wv;
    wv.url.changed.subscribe(url_slot_, url_cb_);
}
void web_view_handler<platform::linux_>::map_html(basic_web_view& wv) {
    wv.html_source.changed.subscribe(html_slot_, html_cb_);
}

} // namespace mpapp::internal
namespace {
GtkWidget* dispatch_web_view_stub(::mpapp::view*) { return nullptr; }
struct registrar_stub {
    registrar_stub() { ::mpapp::detail::linux_dispatch::register_dispatcher(dispatch_web_view_stub); }
};
[[maybe_unused]] registrar_stub _reg_stub;
} // namespace

#endif // __linux__ && !__ANDROID__
