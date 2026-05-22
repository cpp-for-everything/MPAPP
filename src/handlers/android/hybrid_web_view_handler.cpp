// SPDX-License-Identifier: Apache-2.0
// Android hybrid_web_view handler implementation.

#include "mpapp/handlers/android/hybrid_web_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp {

namespace {

jobject make_web_view(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/webkit/WebView");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck() || local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(cls);
        return nullptr;
    }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

void enable_javascript(JNIEnv* env, jobject wv) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass wv_cls = env->FindClass("android/webkit/WebView");
    if (wv_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID get_settings = env->GetMethodID(wv_cls, "getSettings",
        "()Landroid/webkit/WebSettings;");
    if (get_settings == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(wv_cls);
        return;
    }
    jobject settings = env->CallObjectMethod(wv, get_settings);
    env->DeleteLocalRef(wv_cls);
    if (settings == nullptr) { env->ExceptionClear(); return; }

    jclass ws_cls = env->FindClass("android/webkit/WebSettings");
    if (ws_cls != nullptr) {
        jmethodID set_js = env->GetMethodID(ws_cls, "setJavaScriptEnabled", "(Z)V");
        if (set_js != nullptr) {
            env->CallVoidMethod(settings, set_js, JNI_TRUE);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(ws_cls);
    }
    env->DeleteLocalRef(settings);
}

jobject install_js_bridge(JNIEnv* env, jobject wv, jlong handler_ptr) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("io/mpapp/MppJsBridge");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(J)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, handler_ptr);
    if (local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(cls);
        return nullptr;
    }
    // WebView.addJavascriptInterface(bridge, "mpapp_native")
    jclass wv_cls = env->FindClass("android/webkit/WebView");
    if (wv_cls != nullptr) {
        jmethodID add = env->GetMethodID(wv_cls, "addJavascriptInterface",
            "(Ljava/lang/Object;Ljava/lang/String;)V");
        if (add != nullptr) {
            jstring name = env->NewStringUTF("mpapp_native");
            env->CallVoidMethod(wv, add, local, name);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(name);
        }
        env->DeleteLocalRef(wv_cls);
    }
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    env->DeleteLocalRef(cls);
    return global;
}

// JS shim — exposes window.mpapp.{send, on, _receive} where `send`
// routes to the @JavascriptInterface bridge MppJsBridge.send.
constexpr const char* kBridgeShim =
    "javascript:(function(){"
    "  if (window.mpapp && window.mpapp.__mpapp) return;"
    "  var listeners = [];"
    "  window.mpapp = {"
    "    __mpapp: true,"
    "    send: function(p) { if (window.mpapp_native) window.mpapp_native.send(String(p)); },"
    "    on:   function(fn) { listeners.push(fn); },"
    "    _receive: function(p) { for (var i=0; i<listeners.length; ++i) try { listeners[i](p); } catch(e) {} }"
    "  };"
    "})();";

void inject_shim(JNIEnv* env, jobject wv) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/webkit/WebView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "loadUrl", "(Ljava/lang/String;)V");
    if (m != nullptr) {
        jstring s = env->NewStringUTF(kBridgeShim);
        env->CallVoidMethod(wv, m, s);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(s);
    }
    env->DeleteLocalRef(cls);
}

void evaluate_js(JNIEnv* env, jobject wv, const std::string& js) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/webkit/WebView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "evaluateJavascript",
        "(Ljava/lang/String;Landroid/webkit/ValueCallback;)V");
    if (m != nullptr) {
        jstring s = env->NewStringUTF(js.c_str());
        env->CallVoidMethod(wv, m, s, nullptr);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(s);
    }
    env->DeleteLocalRef(cls);
}

std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 16);
    out.push_back('"');
    for (char c : s) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(static_cast<unsigned char>(c)));
                    out += buf;
                } else {
                    out.push_back(c);
                }
        }
    }
    out.push_back('"');
    return out;
}

} // namespace

hybrid_web_view_handler<platform::android>::hybrid_web_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_web_view(env, detail::get_activity());
    if (native_ != nullptr) {
        enable_javascript(env, native_);
    }
}

hybrid_web_view_handler<platform::android>::~hybrid_web_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (bridge_ != nullptr) { env->DeleteGlobalRef(bridge_); bridge_ = nullptr; }
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void hybrid_web_view_handler<platform::android>::send_outbound(const std::string& payload) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    const std::string js = "window.mpapp && window.mpapp._receive(" + json_escape(payload) + ");";
    evaluate_js(env, native_, js);
}

void hybrid_web_view_handler<platform::android>::on_native_inbound(const std::string& payload) {
    if (bound_ == nullptr) return;
    bound_->last_message_in.set(payload);
    bound_->message_received.emit(payload);
}

void hybrid_web_view_handler<platform::android>::map_messages(hybrid_web_view& h) {
    bound_ = &h;
    h.message_sent.subscribe(sent_slot_, sent_cb_);

    if (native_ != nullptr && !wired_) {
        JNIEnv* env = detail::attach_current_thread();
        if (env != nullptr) {
            bridge_ = install_js_bridge(env, native_,
                            reinterpret_cast<jlong>(this));
            inject_shim(env, native_);
            wired_ = true;
        }
    }
}

void android_hybrid_web_view_dispatch_inbound(hybrid_web_view_handler<platform::android>* h,
                                              const std::string& payload) {
    if (h != nullptr) h->on_native_inbound(payload);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_hybrid_web_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::hybrid_web_view*>(v); w && w->has_hwv_handler()) {
        return w->hwv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_hybrid_web_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
