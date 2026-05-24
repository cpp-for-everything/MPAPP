// SPDX-License-Identifier: Apache-2.0
// Android basic_image_cell handler implementation.

#include "mpapp/handlers/android/image_cell_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

constexpr int LINEAR_LAYOUT_HORIZONTAL = 0;
constexpr int LINEAR_LAYOUT_VERTICAL   = 1;
constexpr int VIEW_VISIBLE = 0;
constexpr int VIEW_GONE    = 8;

constexpr int WRAP_CONTENT = -2;

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

void view_set_visibility(JNIEnv* env, jobject view_obj, int v) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setVisibility", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(view_obj, m, static_cast<jint>(v));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void view_set_padding(JNIEnv* env, jobject view_obj, int left, int top, int right, int bottom) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setPadding", "(IIII)V");
    if (m != nullptr) {
        env->CallVoidMethod(view_obj, m,
                            static_cast<jint>(left), static_cast<jint>(top),
                            static_cast<jint>(right), static_cast<jint>(bottom));
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

void image_view_set_layout(JNIEnv* env, jobject child, int width, int height) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass lp_cls = env->FindClass("android/widget/LinearLayout$LayoutParams");
    if (lp_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID ctor = env->GetMethodID(lp_cls, "<init>", "(II)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(lp_cls); return; }
    jobject lp = env->NewObject(lp_cls, ctor,
                                static_cast<jint>(width),
                                static_cast<jint>(height));
    env->DeleteLocalRef(lp_cls);
    if (lp == nullptr) { env->ExceptionClear(); return; }
    jclass view_cls = env->FindClass("android/view/View");
    if (view_cls != nullptr) {
        jmethodID set_lp = env->GetMethodID(view_cls, "setLayoutParams",
            "(Landroid/view/ViewGroup$LayoutParams;)V");
        if (set_lp != nullptr) {
            env->CallVoidMethod(child, set_lp, lp);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(view_cls);
    }
    env->DeleteLocalRef(lp);
}

void apply_linear_weight_lp(JNIEnv* env, jobject child,
                             int width, int height, float weight) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass lp_cls = env->FindClass("android/widget/LinearLayout$LayoutParams");
    if (lp_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID ctor = env->GetMethodID(lp_cls, "<init>", "(IIF)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(lp_cls); return; }
    jobject lp = env->NewObject(lp_cls, ctor,
                                static_cast<jint>(width),
                                static_cast<jint>(height),
                                static_cast<jfloat>(weight));
    env->DeleteLocalRef(lp_cls);
    if (lp == nullptr) { env->ExceptionClear(); return; }
    jclass view_cls = env->FindClass("android/view/View");
    if (view_cls != nullptr) {
        jmethodID set_lp = env->GetMethodID(view_cls, "setLayoutParams",
            "(Landroid/view/ViewGroup$LayoutParams;)V");
        if (set_lp != nullptr) {
            env->CallVoidMethod(child, set_lp, lp);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(view_cls);
    }
    env->DeleteLocalRef(lp);
}

// BitmapFactory.decodeFile — mirrors the existing basic_image handler's loader
// so basic_image_cell behaves identically for filesystem-path basic_image URIs.
jobject decode_file(JNIEnv* env, const std::string& path) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass bf_cls = env->FindClass("android/graphics/BitmapFactory");
    if (bf_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID decode = env->GetStaticMethodID(
        bf_cls, "decodeFile", "(Ljava/lang/String;)Landroid/graphics/Bitmap;");
    if (decode == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(bf_cls); return nullptr; }
    jstring jpath = env->NewStringUTF(path.c_str());
    jobject bmp = env->CallStaticObjectMethod(bf_cls, decode, jpath);
    env->DeleteLocalRef(jpath);
    env->DeleteLocalRef(bf_cls);
    if (env->ExceptionCheck()) { env->ExceptionClear(); if (bmp) env->DeleteLocalRef(bmp); return nullptr; }
    return bmp;
}

void image_view_set_bitmap(JNIEnv* env, jobject iv, jobject bmp_or_null) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ImageView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setImageBitmap", "(Landroid/graphics/Bitmap;)V");
    if (m != nullptr) {
        env->CallVoidMethod(iv, m, bmp_or_null);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

image_cell_handler<platform::android>::image_cell_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject ctx = detail::get_activity();

    native_      = make_object(env, "android/widget/LinearLayout", ctx);
    image_view_  = make_object(env, "android/widget/ImageView",    ctx);
    text_box_    = make_object(env, "android/widget/LinearLayout", ctx);
    text_view_   = make_object(env, "android/widget/TextView",     ctx);
    detail_view_ = make_object(env, "android/widget/TextView",     ctx);

    if (native_ != nullptr) {
        ll_set_orientation(env, native_, LINEAR_LAYOUT_HORIZONTAL);
        view_set_padding(env, native_, 24, 12, 24, 12);
    }
    if (text_box_ != nullptr) ll_set_orientation(env, text_box_, LINEAR_LAYOUT_VERTICAL);
    if (detail_view_ != nullptr) view_set_visibility(env, detail_view_, VIEW_GONE);

    if (image_view_ != nullptr) {
        // ~40dp leading icon size, raw px in v1 (consistent with other Android cells).
        image_view_set_layout(env, image_view_, 80, 80);
        vg_add(env, native_, image_view_);
    }
    if (text_box_ != nullptr) {
        apply_linear_weight_lp(env, text_box_, 0, WRAP_CONTENT, 1.0f);
        vg_add(env, text_box_, text_view_);
        vg_add(env, text_box_, detail_view_);
        vg_add(env, native_, text_box_);
    }
}

image_cell_handler<platform::android>::~image_cell_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (detail_view_ != nullptr) { env->DeleteGlobalRef(detail_view_); detail_view_ = nullptr; }
        if (text_view_   != nullptr) { env->DeleteGlobalRef(text_view_);   text_view_   = nullptr; }
        if (text_box_    != nullptr) { env->DeleteGlobalRef(text_box_);    text_box_    = nullptr; }
        if (image_view_  != nullptr) { env->DeleteGlobalRef(image_view_);  image_view_  = nullptr; }
        if (native_      != nullptr) { env->DeleteGlobalRef(native_);      native_      = nullptr; }
    }
}

void image_cell_handler<platform::android>::apply_text(const std::string& v) {
    if (text_view_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    tv_set_text(env, text_view_, v.c_str());
}

void image_cell_handler<platform::android>::apply_detail(const std::string& v) {
    if (detail_view_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    tv_set_text(env, detail_view_, v.c_str());
    view_set_visibility(env, detail_view_, v.empty() ? VIEW_GONE : VIEW_VISIBLE);
}

void image_cell_handler<platform::android>::apply_image_uri(const std::string& v) {
    if (image_view_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (v.empty()) {
        image_view_set_bitmap(env, image_view_, nullptr);
        return;
    }
    if (jobject bmp = decode_file(env, v); bmp != nullptr) {
        image_view_set_bitmap(env, image_view_, bmp);
        env->DeleteLocalRef(bmp);
    }
}

void image_cell_handler<platform::android>::map_text(basic_image_cell& c) {
    apply_text(c.text.get());
    c.text.changed.subscribe(text_slot_, text_cb_);
}

void image_cell_handler<platform::android>::map_detail(basic_image_cell& c) {
    apply_detail(c.detail.get());
    c.detail.changed.subscribe(detail_slot_, detail_cb_);
}

void image_cell_handler<platform::android>::map_image_uri(basic_image_cell& c) {
    apply_image_uri(c.image_uri.get());
    c.image_uri.changed.subscribe(uri_slot_, uri_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_image_cell(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::internal::basic_image_cell*>(v); c && c->has_ic_handler()) {
        return c->ic_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_image_cell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
