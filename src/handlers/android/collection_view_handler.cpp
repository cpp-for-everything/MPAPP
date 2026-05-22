// SPDX-License-Identifier: Apache-2.0
// Android collection_view handler implementation.

#include "mpapp/handlers/android/collection_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp {

namespace {

constexpr jint ANDROID_R_LAYOUT_SIMPLE_LIST_ITEM_1 = 0x01090003;
constexpr jint CHOICE_MODE_SINGLE                  = 1;
constexpr jint CHOICE_MODE_MULTIPLE                = 2;
constexpr jint CHOICE_MODE_NONE                    = 0;

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

void install_adapter(JNIEnv* env, jobject context, jobject list_view,
                     const std::vector<std::string>& items) {
    if (env->ExceptionCheck()) env->ExceptionClear();
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
    jmethodID m = env->GetMethodID(cls, "setSelection", "(I)V");
    if (m != nullptr && idx >= 0) {
        env->CallVoidMethod(list_view, m, static_cast<jint>(idx));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void list_view_set_choice_mode(JNIEnv* env, jobject list_view, jint mode) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ListView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setChoiceMode", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(list_view, m, mode);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

collection_view_handler<platform::android>::collection_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_list_view(env, detail::get_activity());
    if (native_ != nullptr) list_view_set_choice_mode(env, native_, CHOICE_MODE_SINGLE);
}

collection_view_handler<platform::android>::~collection_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void collection_view_handler<platform::android>::rebuild_items(const std::vector<std::string>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    install_adapter(env, detail::get_activity(), native_, v);
    if (bound_ != nullptr) apply_selection(bound_->selected_index.get());
}

void collection_view_handler<platform::android>::apply_selection(int idx) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    list_view_set_selection(env, native_, idx);
}

void collection_view_handler<platform::android>::apply_selection_mode(collection_selection_mode m) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jint mode = CHOICE_MODE_SINGLE;
    switch (m) {
        case collection_selection_mode::none:     mode = CHOICE_MODE_NONE;     break;
        case collection_selection_mode::multiple: mode = CHOICE_MODE_MULTIPLE; break;
        case collection_selection_mode::single:
        default:                                  mode = CHOICE_MODE_SINGLE;   break;
    }
    list_view_set_choice_mode(env, native_, mode);
}

void collection_view_handler<platform::android>::refresh_multi_selection_from_native() {
    if (native_ == nullptr || bound_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();

    // SparseBooleanArray ListView.getCheckedItemPositions()
    jclass lv_cls = env->FindClass("android/widget/ListView");
    if (lv_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID get_checked = env->GetMethodID(lv_cls, "getCheckedItemPositions",
        "()Landroid/util/SparseBooleanArray;");
    jobject sparse = (get_checked != nullptr) ? env->CallObjectMethod(native_, get_checked) : nullptr;
    env->DeleteLocalRef(lv_cls);
    if (sparse == nullptr) { env->ExceptionClear(); return; }

    jclass sba_cls = env->FindClass("android/util/SparseBooleanArray");
    if (sba_cls == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(sparse);
        return;
    }
    jmethodID size_m = env->GetMethodID(sba_cls, "size",    "()I");
    jmethodID key_at = env->GetMethodID(sba_cls, "keyAt",   "(I)I");
    jmethodID val_at = env->GetMethodID(sba_cls, "valueAt", "(I)Z");
    std::vector<int> idxs;
    if (size_m != nullptr && key_at != nullptr && val_at != nullptr) {
        const jint n = env->CallIntMethod(sparse, size_m);
        idxs.reserve(static_cast<std::size_t>(n));
        for (jint i = 0; i < n; ++i) {
            jboolean v = env->CallBooleanMethod(sparse, val_at, i);
            if (v == JNI_TRUE) {
                jint k = env->CallIntMethod(sparse, key_at, i);
                idxs.push_back(static_cast<int>(k));
            }
        }
    }
    env->DeleteLocalRef(sba_cls);
    env->DeleteLocalRef(sparse);
    if (env->ExceptionCheck()) env->ExceptionClear();

    if (bound_->selected_indices.get() != idxs) {
        bound_->selected_indices.set(std::move(idxs));
    }
}

namespace {

// Install MppItemClickRouter(collection_view*, kind=1) — same shape as
// the list_view router, different kind code.
void install_item_click_router(JNIEnv* env, jobject list_view_obj, collection_view* cv) {
    if (env == nullptr || list_view_obj == nullptr || cv == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass router_cls = env->FindClass("io/mpapp/MppItemClickRouter");
    if (router_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID ctor = env->GetMethodID(router_cls, "<init>", "(JI)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(router_cls); return; }
    jobject router = env->NewObject(router_cls, ctor,
                                    reinterpret_cast<jlong>(cv),
                                    static_cast<jint>(1 /* collection_view kind */));
    if (env->ExceptionCheck() || router == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(router_cls);
        return;
    }
    env->DeleteLocalRef(router_cls);

    jclass av_cls = env->FindClass("android/widget/AdapterView");
    if (av_cls != nullptr) {
        jmethodID set_listener = env->GetMethodID(av_cls, "setOnItemClickListener",
            "(Landroid/widget/AdapterView$OnItemClickListener;)V");
        if (set_listener != nullptr) {
            env->CallVoidMethod(list_view_obj, set_listener, router);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(av_cls);
    }
    env->DeleteLocalRef(router);
}

} // namespace

void collection_view_handler<platform::android>::map_items_source(collection_view& cv) {
    bound_ = &cv;
    rebuild_items(cv.items_source.get());
    cv.items_source.changed.subscribe(items_slot_, items_cb_);
    JNIEnv* env = detail::attach_current_thread();
    if (env != nullptr) install_item_click_router(env, native_, &cv);
}

void collection_view_handler<platform::android>::map_selected_index(collection_view& cv) {
    apply_selection(cv.selected_index.get());
    cv.selected_index.changed.subscribe(sel_slot_, sel_cb_);
}

void collection_view_handler<platform::android>::map_selection_mode(collection_view& cv) {
    apply_selection_mode(cv.selection_mode.get());
    cv.selection_mode.changed.subscribe(mode_slot_, mode_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_collection_view(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::collection_view*>(v); c && c->has_cv_handler()) {
        return c->cv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_collection_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
