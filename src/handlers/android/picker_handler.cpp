// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android picker handler implementation.

#include "mpapp/handlers/android/picker_handler.hpp"

#if defined(__ANDROID__)

#include <string>
#include <vector>

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

namespace {

jobject make_spinner(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/Spinner");
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

// Build an ArrayAdapter<String> with the simple_spinner_item layout and
// the given strings. Returns local ref (caller must DeleteLocalRef).
jobject make_array_adapter(JNIEnv* env, jobject context,
                           const std::vector<std::string>& items) {
    if (env->ExceptionCheck()) env->ExceptionClear();

    // jstring[] of items
    jclass string_cls = env->FindClass("java/lang/String");
    if (string_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jobjectArray arr = env->NewObjectArray(
        static_cast<jsize>(items.size()), string_cls, nullptr);
    if (arr == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(string_cls); return nullptr; }
    for (size_t i = 0; i < items.size(); ++i) {
        jstring js = env->NewStringUTF(items[i].c_str());
        env->SetObjectArrayElement(arr, static_cast<jsize>(i), js);
        env->DeleteLocalRef(js);
    }
    env->DeleteLocalRef(string_cls);

    // ArrayAdapter.<init>(Context, int resource, Object[])
    jclass adapter_cls = env->FindClass("android/widget/ArrayAdapter");
    if (adapter_cls == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(arr);
        return nullptr;
    }
    jmethodID ctor = env->GetMethodID(
        adapter_cls, "<init>", "(Landroid/content/Context;I[Ljava/lang/Object;)V");
    if (ctor == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(adapter_cls);
        env->DeleteLocalRef(arr);
        return nullptr;
    }
    // android.R.layout.simple_spinner_item = 0x01090008
    constexpr jint kSimpleSpinnerItem = 0x01090008;
    jobject adapter = env->NewObject(adapter_cls, ctor,
                                     context, kSimpleSpinnerItem, arr);
    env->DeleteLocalRef(arr);
    env->DeleteLocalRef(adapter_cls);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        if (adapter) env->DeleteLocalRef(adapter);
        return nullptr;
    }
    return adapter;
}

} // namespace

picker_handler<platform::android>::picker_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_spinner(env, detail::get_activity());
}

picker_handler<platform::android>::~picker_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void picker_handler<platform::android>::apply_items(const std::vector<std::string>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();

    jobject adapter = make_array_adapter(env, detail::get_activity(), v);
    if (adapter == nullptr) return;

    jclass cls = env->FindClass("android/widget/Spinner");
    if (cls != nullptr) {
        jmethodID set_adapter = env->GetMethodID(
            cls, "setAdapter", "(Landroid/widget/SpinnerAdapter;)V");
        if (set_adapter != nullptr) {
            suppress_echo_ = true;
            env->CallVoidMethod(native_, set_adapter, adapter);
            if (env->ExceptionCheck()) env->ExceptionClear();
            suppress_echo_ = false;
        }
        env->DeleteLocalRef(cls);
    }
    env->DeleteLocalRef(adapter);
}

void picker_handler<platform::android>::apply_selected_index(int v) {
    if (native_ == nullptr || v < 0) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/AbsSpinner");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID set_sel = env->GetMethodID(cls, "setSelection", "(I)V");
    if (set_sel != nullptr) {
        suppress_echo_ = true;
        env->CallVoidMethod(native_, set_sel, static_cast<jint>(v));
        if (env->ExceptionCheck()) env->ExceptionClear();
        suppress_echo_ = false;
    }
    env->DeleteLocalRef(cls);
}

void picker_handler<platform::android>::map_items(picker& p) {
    apply_items(p.items.get());
    p.items.changed.subscribe(items_slot_, items_cb_);
}
void picker_handler<platform::android>::map_selected_index(picker& p) {
    apply_selected_index(p.selected_index.get());
    p.selected_index.changed.subscribe(selected_slot_, selected_cb_);
}
void picker_handler<platform::android>::map_title(picker& p) {
    apply_title(p.title.get());
    p.title.changed.subscribe(title_slot_, title_cb_);
}

} // namespace mpapp

#endif // __ANDROID__
