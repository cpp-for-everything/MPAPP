// SPDX-License-Identifier: Apache-2.0
// Android basic_table_view handler implementation.

#include "mpapp/handlers/android/table_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/cell.hpp"
#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

constexpr jint ANDROID_R_LAYOUT_SIMPLE_LIST_ITEM_1 = 0x01090003;
constexpr int  LINEAR_LAYOUT_VERTICAL = 1;
constexpr int  MATCH_PARENT = -1;
constexpr int  WRAP_CONTENT = -2;

constexpr jint COLOR_HEADER = static_cast<jint>(0xFF1976D2u);

jobject make_object(JNIEnv* env, const char* cls_name, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass(cls_name);
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

void ll_set_orientation(JNIEnv* env, jobject ll, int orientation) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/LinearLayout");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setOrientation", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(ll, m, static_cast<jint>(orientation));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void tv_set_text(JNIEnv* env, jobject tv, const char* text) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setText", "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring s = env->NewStringUTF(text);
        env->CallVoidMethod(tv, m, s);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(s);
    }
    env->DeleteLocalRef(cls);
}

void tv_set_text_color(JNIEnv* env, jobject tv, jint argb) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setTextColor", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(tv, m, argb);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

jobject typeface_default_bold(JNIEnv* env) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/graphics/Typeface");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jfieldID fid = env->GetStaticFieldID(cls, "DEFAULT_BOLD", "Landroid/graphics/Typeface;");
    jobject tf = (fid != nullptr) ? env->GetStaticObjectField(cls, fid) : nullptr;
    env->DeleteLocalRef(cls);
    return tf;
}

void tv_set_typeface(JNIEnv* env, jobject tv, jobject typeface) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setTypeface", "(Landroid/graphics/Typeface;)V");
    if (m != nullptr) {
        env->CallVoidMethod(tv, m, typeface);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void view_set_padding(JNIEnv* env, jobject view_obj, int l, int t, int r, int b) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setPadding", "(IIII)V");
    if (m != nullptr) {
        env->CallVoidMethod(view_obj, m,
                            static_cast<jint>(l), static_cast<jint>(t),
                            static_cast<jint>(r), static_cast<jint>(b));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void vg_add(JNIEnv* env, jobject parent, jobject child) {
    if (child == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID add = env->GetMethodID(cls, "addView", "(Landroid/view/View;)V");
    if (add != nullptr) {
        env->CallVoidMethod(parent, add, child);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void vg_remove_all(JNIEnv* env, jobject parent) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "removeAllViews", "()V");
    if (m != nullptr) {
        env->CallVoidMethod(parent, m);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

jobject make_list_view(JNIEnv* env, jobject context) {
    return make_object(env, "android/widget/ListView", context);
}

void install_string_adapter(JNIEnv* env, jobject context, jobject basic_list_view,
                             const std::vector<std::string>& items) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass string_cls = env->FindClass("java/lang/String");
    if (string_cls == nullptr) { env->ExceptionClear(); return; }
    jobjectArray arr = env->NewObjectArray(static_cast<jsize>(items.size()), string_cls, nullptr);
    if (arr == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(string_cls); return; }
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
                                     ANDROID_R_LAYOUT_SIMPLE_LIST_ITEM_1, arr);
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
            env->CallVoidMethod(basic_list_view, set_adapter, adapter);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(lv_cls);
    }
    env->DeleteLocalRef(adapter);
}

// Install MppItemClickRouter(basic_table_view*, kind=2) on the ListView's
// OnItemClickListener slot — user taps then resolve back to (section,
// row) and emit row_tapped via item_click_router.cpp.
void install_item_click_router(JNIEnv* env, jobject list_view_obj, basic_table_view* tv) {
    if (env == nullptr || list_view_obj == nullptr || tv == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass router_cls = env->FindClass("io/mpapp/MppItemClickRouter");
    if (router_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID ctor = env->GetMethodID(router_cls, "<init>", "(JI)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(router_cls); return; }
    jobject router = env->NewObject(router_cls, ctor,
                                    reinterpret_cast<jlong>(tv),
                                    static_cast<jint>(2 /* basic_table_view kind */));
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

table_view_handler<platform::android>::table_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    // Outer FrameLayout is the stable native handle. Inner widget swaps
    // between flat-mode ListView and typed-mode ScrollView.
    native_ = make_object(env, "android/widget/FrameLayout", detail::get_activity());
}

table_view_handler<platform::android>::~table_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (list_view_  != nullptr) { env->DeleteGlobalRef(list_view_);  list_view_  = nullptr; }
        if (typed_root_ != nullptr) { env->DeleteGlobalRef(typed_root_); typed_root_ = nullptr; }
        if (native_     != nullptr) { env->DeleteGlobalRef(native_);     native_     = nullptr; }
    }
}

void table_view_handler<platform::android>::rebuild_items(const std::vector<table_section_data>& sections) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    // Clear outer; release any prior typed_root.
    vg_remove_all(env, native_);
    if (typed_root_ != nullptr) { env->DeleteGlobalRef(typed_root_); typed_root_ = nullptr; }

    // Create or reuse the flat ListView.
    if (list_view_ == nullptr) {
        list_view_ = make_list_view(env, detail::get_activity());
    }

    std::vector<std::string> flat;
    flat.reserve(sections.size() * 4);
    for (const auto& sec : sections) {
        flat.push_back("\xe2\x96\xbe " + sec.title);  // "▾ " + title
        for (const auto& row : sec.rows) flat.push_back(row);
    }
    install_string_adapter(env, detail::get_activity(), list_view_, flat);
    if (bound_ != nullptr) install_item_click_router(env, list_view_, bound_);
    vg_add(env, native_, list_view_);
}

void table_view_handler<platform::android>::rebuild_typed(const std::vector<table_section_typed>& sections) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    vg_remove_all(env, native_);
    // The flat list_view_ stays around — its parent is detached, so it's
    // safe to keep the global ref for the next flat-mode rebuild.

    jobject ctx = detail::get_activity();
    typed_root_ = make_object(env, "android/widget/ScrollView", ctx);
    jobject column = make_object(env, "android/widget/LinearLayout", ctx);
    ll_set_orientation(env, column, LINEAR_LAYOUT_VERTICAL);

    jobject tf_bold = typeface_default_bold(env);
    for (const auto& sec : sections) {
        // Section header — bold, primary-blue, leading "▾ ".
        jobject hdr = make_object(env, "android/widget/TextView", ctx);
        if (hdr != nullptr) {
            const std::string basic_label = "\xe2\x96\xbe " + sec.title;
            tv_set_text(env, hdr, basic_label.c_str());
            tv_set_text_color(env, hdr, COLOR_HEADER);
            if (tf_bold != nullptr) tv_set_typeface(env, hdr, tf_bold);
            view_set_padding(env, hdr, 24, 16, 24, 8);
            vg_add(env, column, hdr);
            env->DeleteGlobalRef(hdr);
        }
        for (cell* c : sec.rows) {
            if (c == nullptr) continue;
            jobject native = detail::android_dispatch::dispatch(c);
            if (native != nullptr) {
                vg_add(env, column, native);
            }
        }
    }
    if (tf_bold != nullptr) env->DeleteLocalRef(tf_bold);

    vg_add(env, typed_root_, column);
    env->DeleteGlobalRef(column);
    vg_add(env, native_, typed_root_);
}

void table_view_handler<platform::android>::rebuild_active() {
    if (bound_ == nullptr) return;
    const auto& typed = bound_->typed_sections.get();
    if (!typed.empty()) {
        rebuild_typed(typed);
    } else {
        rebuild_items(bound_->sections.get());
    }
}

void table_view_handler<platform::android>::apply_row_height(int /*h*/) {
    // row_height honoring requires a custom adapter overriding getView's
    // setLayoutParams; not wired in v1.
}

void table_view_handler<platform::android>::map_sections(basic_table_view& tv) {
    bound_ = &tv;
    rebuild_active();
    tv.sections.changed.subscribe(sec_slot_, sec_cb_);
}

void table_view_handler<platform::android>::map_typed_sections(basic_table_view& tv) {
    bound_ = &tv;
    rebuild_active();
    tv.typed_sections.changed.subscribe(typed_slot_, typed_cb_);
}

void table_view_handler<platform::android>::map_row_height(basic_table_view& tv) {
    apply_row_height(tv.row_height.get());
    tv.row_height.changed.subscribe(rh_slot_, rh_cb_);
}

// Suppress -Wunused warnings on MATCH_PARENT / WRAP_CONTENT — kept for
// readability + symmetry with sibling Android handlers.
[[maybe_unused]] static constexpr int _suppress1 = MATCH_PARENT;
[[maybe_unused]] static constexpr int _suppress2 = WRAP_CONTENT;

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_table_view(::mpapp::view* v) {
    if (auto* t = dynamic_cast<::mpapp::internal::basic_table_view*>(v); t && t->has_tv_handler()) {
        return t->tv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_table_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
