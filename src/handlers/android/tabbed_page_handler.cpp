// SPDX-License-Identifier: Apache-2.0
// Android basic_tabbed_page handler implementation.

#include "mpapp/handlers/android/tabbed_page_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/internal/basic_page.hpp"

namespace mpapp::internal {

namespace {

constexpr int LINEAR_LAYOUT_VERTICAL   = 1;
constexpr int LINEAR_LAYOUT_HORIZONTAL = 0;

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

// android.graphics.Typeface.DEFAULT / DEFAULT_BOLD via static field access.
jobject typeface_default(JNIEnv* env, bool bold) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/graphics/Typeface");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jfieldID fid = env->GetStaticFieldID(cls,
        bold ? "DEFAULT_BOLD" : "DEFAULT", "Landroid/graphics/Typeface;");
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

// Install MppActionRouter(tp, kind=2, payload=tab_index) on a tab TextView.
void install_tab_click_router(JNIEnv* env, jobject tab_view,
                              jlong owner_ptr, jint tab_index) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass router_cls = env->FindClass("io/mpapp/MppActionRouter");
    if (router_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID ctor = env->GetMethodID(router_cls, "<init>", "(JII)V");
    if (ctor == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(router_cls);
        return;
    }
    jobject router = env->NewObject(router_cls, ctor,
                                    owner_ptr,
                                    static_cast<jint>(2 /* tabbed_page_tab kind */),
                                    tab_index);
    if (env->ExceptionCheck() || router == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(router_cls);
        return;
    }
    env->DeleteLocalRef(router_cls);

    jclass view_cls = env->FindClass("android/view/View");
    if (view_cls != nullptr) {
        jmethodID set_l = env->GetMethodID(view_cls, "setOnClickListener",
            "(Landroid/view/View$OnClickListener;)V");
        if (set_l != nullptr) {
            env->CallVoidMethod(tab_view, set_l, router);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(view_cls);
    }
    env->DeleteLocalRef(router);
}

// Material-ish color hints: 0xFF1976D2 = primary blue, 0xFF606060 = grey.
constexpr jint COLOR_SELECTED   = static_cast<jint>(0xFF1976D2u);
constexpr jint COLOR_UNSELECTED = static_cast<jint>(0xFF606060u);

} // namespace

tabbed_page_handler<platform::android>::tabbed_page_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject ctx = detail::get_activity();

    native_       = make_object(env, "android/widget/LinearLayout", ctx);
    tab_strip_    = make_object(env, "android/widget/LinearLayout", ctx);
    content_host_ = make_object(env, "android/widget/FrameLayout",  ctx);

    if (native_    != nullptr) ll_set_orientation(env, native_,    LINEAR_LAYOUT_VERTICAL);
    if (tab_strip_ != nullptr) ll_set_orientation(env, tab_strip_, LINEAR_LAYOUT_HORIZONTAL);

    if (native_ != nullptr) {
        vg_add(env, native_, tab_strip_);
        vg_add(env, native_, content_host_);
    }
}

tabbed_page_handler<platform::android>::~tabbed_page_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        for (jobject t : tab_views_) {
            if (t != nullptr) env->DeleteGlobalRef(t);
        }
        tab_views_.clear();
        if (content_host_ != nullptr) { env->DeleteGlobalRef(content_host_); content_host_ = nullptr; }
        if (tab_strip_    != nullptr) { env->DeleteGlobalRef(tab_strip_);    tab_strip_    = nullptr; }
        if (native_       != nullptr) { env->DeleteGlobalRef(native_);       native_       = nullptr; }
    }
}

void tabbed_page_handler<platform::android>::rebuild_children(const std::vector<basic_page*>& kids) {
    current_kids_ = kids;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    // Release any previous tab-view global refs before clearing the strip.
    for (jobject t : tab_views_) {
        if (t != nullptr) env->DeleteGlobalRef(t);
    }
    tab_views_.clear();

    vg_remove_all(env, tab_strip_);
    jobject ctx = detail::get_activity();
    for (std::size_t i = 0; i < kids.size(); ++i) {
        basic_page* p = kids[i];
        if (p == nullptr) {
            tab_views_.push_back(nullptr);
            continue;
        }
        jobject basic_label = make_object(env, "android/widget/TextView", ctx);
        if (basic_label != nullptr) {
            tv_set_text(env, basic_label, p->title.get().c_str());
            view_set_padding(env, basic_label, 32, 16, 32, 16);
            install_tab_click_router(env, basic_label,
                                     reinterpret_cast<jlong>(bound_),
                                     static_cast<jint>(i));
            vg_add(env, tab_strip_, basic_label);
            // Keep a strong (global) ref so apply_selection can restyle it.
            tab_views_.push_back(basic_label);
        } else {
            tab_views_.push_back(nullptr);
        }
    }
    // Render current selection content + restyle tabs.
    if (bound_ != nullptr) apply_selection(bound_->selected_index.get());
}

void tabbed_page_handler<platform::android>::apply_selection(int idx) {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    // Restyle every tab: selected gets primary color + bold, others get
    // grey + regular. The selected-index out-of-range case (e.g. empty
    // children) restyles all tabs as unselected.
    jobject tf_bold    = typeface_default(env, true);
    jobject tf_regular = typeface_default(env, false);
    for (std::size_t i = 0; i < tab_views_.size(); ++i) {
        jobject t = tab_views_[i];
        if (t == nullptr) continue;
        const bool is_selected = (static_cast<int>(i) == idx);
        tv_set_text_color(env, t, is_selected ? COLOR_SELECTED : COLOR_UNSELECTED);
        tv_set_typeface(env, t, is_selected ? tf_bold : tf_regular);
    }
    if (tf_bold    != nullptr) env->DeleteLocalRef(tf_bold);
    if (tf_regular != nullptr) env->DeleteLocalRef(tf_regular);

    // Swap content area.
    vg_remove_all(env, content_host_);
    if (idx < 0 || idx >= static_cast<int>(current_kids_.size())) return;
    basic_page* sel = current_kids_[static_cast<std::size_t>(idx)];
    if (sel == nullptr) return;
    if (jobject native = detail::android_dispatch::dispatch(sel); native != nullptr) {
        vg_add(env, content_host_, native);
    }
}

void tabbed_page_handler<platform::android>::map_children(basic_tabbed_page& tp) {
    bound_ = &tp;
    rebuild_children(tp.children.get());
    tp.children.changed.subscribe(children_slot_, children_cb_);
}

void tabbed_page_handler<platform::android>::map_selected_index(basic_tabbed_page& tp) {
    apply_selection(tp.selected_index.get());
    tp.selected_index.changed.subscribe(selection_slot_, selection_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_tabbed_page(::mpapp::view* v) {
    if (auto* t = dynamic_cast<::mpapp::internal::basic_tabbed_page*>(v); t && t->has_tp_handler()) {
        return t->tp_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_tabbed_page); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
