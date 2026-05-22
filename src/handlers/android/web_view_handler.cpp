// SPDX-License-Identifier: Apache-2.0
// Android web_view handler implementation.

#include "mpapp/handlers/android/web_view_handler.hpp"

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

void web_view_enable_javascript(JNIEnv* env, jobject wv) {
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

void web_view_load_url(JNIEnv* env, jobject wv, const std::string& url) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/webkit/WebView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "loadUrl", "(Ljava/lang/String;)V");
    if (m != nullptr) {
        jstring s = env->NewStringUTF(url.c_str());
        env->CallVoidMethod(wv, m, s);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(s);
    }
    env->DeleteLocalRef(cls);
}

void web_view_load_html(JNIEnv* env, jobject wv, const std::string& html) {
    // loadDataWithBaseURL(null, html, "text/html", "utf-8", null) — gives
    // the loaded page a stable origin for relative URL resolution.
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/webkit/WebView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "loadDataWithBaseURL",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    if (m != nullptr) {
        jstring jhtml = env->NewStringUTF(html.c_str());
        jstring jmime = env->NewStringUTF("text/html");
        jstring jenc  = env->NewStringUTF("utf-8");
        env->CallVoidMethod(wv, m, nullptr, jhtml, jmime, jenc, nullptr);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(jhtml);
        env->DeleteLocalRef(jmime);
        env->DeleteLocalRef(jenc);
    }
    env->DeleteLocalRef(cls);
}

bool web_view_can_go(JNIEnv* env, jobject wv, bool forward) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/webkit/WebView");
    if (cls == nullptr) { env->ExceptionClear(); return false; }
    jmethodID m = env->GetMethodID(cls, forward ? "canGoForward" : "canGoBack", "()Z");
    bool out = false;
    if (m != nullptr) {
        jboolean r = env->CallBooleanMethod(wv, m);
        out = (r == JNI_TRUE);
        if (env->ExceptionCheck()) { env->ExceptionClear(); out = false; }
    }
    env->DeleteLocalRef(cls);
    return out;
}

jobject install_web_view_client(JNIEnv* env, jobject wv, jlong handler_ptr) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass client_cls = env->FindClass("io/mpapp/MppWebViewClient");
    if (client_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(client_cls, "<init>", "(J)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(client_cls); return nullptr; }
    jobject local = env->NewObject(client_cls, ctor, handler_ptr);
    if (local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(client_cls);
        return nullptr;
    }
    jclass wv_cls = env->FindClass("android/webkit/WebView");
    if (wv_cls != nullptr) {
        jmethodID set_client = env->GetMethodID(wv_cls, "setWebViewClient",
            "(Landroid/webkit/WebViewClient;)V");
        if (set_client != nullptr) {
            env->CallVoidMethod(wv, set_client, local);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(wv_cls);
    }
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    env->DeleteLocalRef(client_cls);
    return global;
}

} // namespace

web_view_handler<platform::android>::web_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_web_view(env, detail::get_activity());
    if (native_ != nullptr) {
        web_view_enable_javascript(env, native_);
    }
}

web_view_handler<platform::android>::~web_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (client_ != nullptr) { env->DeleteGlobalRef(client_); client_ = nullptr; }
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void web_view_handler<platform::android>::apply_url(const std::string& v) {
    if (native_ == nullptr || v.empty()) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    suppress_echo_ = true;
    web_view_load_url(env, native_, v);
    suppress_echo_ = false;
}

void web_view_handler<platform::android>::apply_html(const std::string& v) {
    if (native_ == nullptr || v.empty()) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    web_view_load_html(env, native_, v);
}

void web_view_handler<platform::android>::refresh_can_go() {
    if (native_ == nullptr || bound_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    bound_->can_go_back.set(web_view_can_go(env, native_, false));
    bound_->can_go_forward.set(web_view_can_go(env, native_, true));
}

void web_view_handler<platform::android>::map_url(web_view& wv) {
    bound_ = &wv;
    apply_url(wv.url.get());
    wv.url.changed.subscribe(url_slot_, url_cb_);

    if (native_ != nullptr && client_ == nullptr) {
        JNIEnv* env = detail::attach_current_thread();
        if (env != nullptr) {
            client_ = install_web_view_client(env, native_,
                            reinterpret_cast<jlong>(this));
        }
    }
}

void web_view_handler<platform::android>::map_html(web_view& wv) {
    apply_html(wv.html_source.get());
    wv.html_source.changed.subscribe(html_slot_, html_cb_);
}

void web_view_handler<platform::android>::on_native_page_started(const std::string& url) {
    if (bound_ == nullptr) return;
    bound_->is_loading.set(true);
    bound_->navigating.emit(url);
}

void web_view_handler<platform::android>::on_native_page_finished(const std::string& url, bool success) {
    if (bound_ == nullptr) return;
    bound_->is_loading.set(false);
    refresh_can_go();
    bound_->navigated.emit(url, success);
}

void android_web_view_dispatch_page_started(web_view_handler<platform::android>* h,
                                            const std::string& url) {
    if (h != nullptr) h->on_native_page_started(url);
}

void android_web_view_dispatch_page_finished(web_view_handler<platform::android>* h,
                                             const std::string& url,
                                             bool success) {
    if (h != nullptr) h->on_native_page_finished(url, success);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_web_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::web_view*>(v); w && w->has_wv_handler()) {
        return w->wv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_web_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
