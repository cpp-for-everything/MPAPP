// SPDX-License-Identifier: Apache-2.0
// Android list_view handler implementation.
// Wraps android.widget.ListView with an ArrayAdapter<String>.

#include "mpapp/handlers/android/list_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp {

namespace {

// Resource ID for android.R.layout.simple_list_item_1. Looking it up via
// android.R is JNI-heavy; use the literal documented in the Android
// public API (stable since API 1).
constexpr jint ANDROID_R_LAYOUT_SIMPLE_LIST_ITEM_1 = 0x01090003 /* hex */;

jobject make_list_view(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ListView");
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

// Build a fresh ArrayAdapter<String> populated from `items` and attach
// it to the ListView via setAdapter. Each refresh replaces the adapter.
void install_adapter(JNIEnv* env, jobject context, jobject list_view,
                     const std::vector<std::string>& items) {
    if (env->ExceptionCheck()) env->ExceptionClear();

    // Build a Java String[] from items.
    jclass string_cls = env->FindClass("java/lang/String");
    if (string_cls == nullptr) { env->ExceptionClear(); return; }
    jobjectArray arr = env->NewObjectArray(static_cast<jsize>(items.size()), string_cls, nullptr);
    if (arr == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(string_cls);
        return;
    }
    for (jsize i = 0; i < static_cast<jsize>(items.size()); ++i) {
        jstring s = env->NewStringUTF(items[static_cast<std::size_t>(i)].c_str());
        env->SetObjectArrayElement(arr, i, s);
        env->DeleteLocalRef(s);
    }
    env->DeleteLocalRef(string_cls);

    // Construct ArrayAdapter<String>(context, simple_list_item_1, arr).
    jclass adapter_cls = env->FindClass("android/widget/ArrayAdapter");
    if (adapter_cls == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(arr); return; }
    jmethodID ctor = env->GetMethodID(adapter_cls, "<init>",
        "(Landroid/content/Context;I[Ljava/lang/Object;)V");
    if (ctor == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(adapter_cls);
        env->DeleteLocalRef(arr);
        return;
    }
    jobject adapter = env->NewObject(adapter_cls, ctor, context,
                                     ANDROID_R_LAYOUT_SIMPLE_LIST_ITEM_1,
                                     arr);
    env->DeleteLocalRef(arr);
    if (env->ExceptionCheck() || adapter == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(adapter_cls);
        return;
    }
    env->DeleteLocalRef(adapter_cls);

    // listView.setAdapter(adapter)
    jclass lv_cls = env->FindClass("android/widget/ListView");
    if (lv_cls != nullptr) {
        jmethodID set_adapter = env->GetMethodID(lv_cls, "setAdapter",
            "(Landroid/widget/ListAdapter;)V");
        if (set_adapter != nullptr) {
            env->CallVoidMethod(list_view, set_adapter, adapter);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(lv_cls);
    }
    env->DeleteLocalRef(adapter);
}

void list_view_set_selection(JNIEnv* env, jobject list_view, int idx) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ListView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    // ListView.setSelection(int)
    jmethodID m = env->GetMethodID(cls, "setSelection", "(I)V");
    if (m != nullptr && idx >= 0) {
        env->CallVoidMethod(list_view, m, static_cast<jint>(idx));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

list_view_handler<platform::android>::list_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject ctx = detail::get_activity();
    native_ = make_list_view(env, ctx);
    // OnItemClickListener wiring to set selected_index + emit
    // item_tapped is deferred to M-05 polish — consistent with the
    // navigation_page and shell tab-button click wiring.
}

list_view_handler<platform::android>::~list_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void list_view_handler<platform::android>::rebuild_items(const std::vector<std::string>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    install_adapter(env, detail::get_activity(), native_, v);
    if (bound_ != nullptr) apply_selection(bound_->selected_index.get());
}

void list_view_handler<platform::android>::apply_selection(int idx) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    list_view_set_selection(env, native_, idx);
}

void list_view_handler<platform::android>::map_items_source(list_view& lv) {
    bound_ = &lv;
    rebuild_items(lv.items_source.get());
    lv.items_source.changed.subscribe(items_slot_, items_cb_);
}

void list_view_handler<platform::android>::map_selected_index(list_view& lv) {
    apply_selection(lv.selected_index.get());
    lv.selected_index.changed.subscribe(sel_slot_, sel_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_list_view(::mpapp::view* v) {
    if (auto* l = dynamic_cast<::mpapp::list_view*>(v); l && l->has_lv_handler()) {
        return l->lv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_list_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
