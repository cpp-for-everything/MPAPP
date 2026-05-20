// SPDX-License-Identifier: Apache-2.0
// Android image_button handler implementation.

#include "mpapp/handlers/android/image_button_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

namespace {

jobject make_image_button(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ImageButton");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck() || local == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

jobject decode_file(JNIEnv* env, const std::string& path) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass bf = env->FindClass("android/graphics/BitmapFactory");
    if (bf == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID decode = env->GetStaticMethodID(bf, "decodeFile", "(Ljava/lang/String;)Landroid/graphics/Bitmap;");
    if (decode == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(bf); return nullptr; }
    jstring jpath = env->NewStringUTF(path.c_str());
    jobject bmp = env->CallStaticObjectMethod(bf, decode, jpath);
    env->DeleteLocalRef(jpath);
    env->DeleteLocalRef(bf);
    if (env->ExceptionCheck()) { env->ExceptionClear(); if (bmp) env->DeleteLocalRef(bmp); return nullptr; }
    return bmp;
}

} // namespace

image_button_handler<platform::android>::image_button_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_image_button(env, detail::get_activity());
}

image_button_handler<platform::android>::~image_button_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void image_button_handler<platform::android>::apply_source(const std::string& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    // ImageButton extends ImageView; setImageBitmap lives on ImageView.
    jclass cls = env->FindClass("android/widget/ImageView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID set_bmp = env->GetMethodID(cls, "setImageBitmap", "(Landroid/graphics/Bitmap;)V");
    if (v.empty()) {
        if (set_bmp != nullptr) {
            env->CallVoidMethod(native_, set_bmp, nullptr);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
    } else {
        jobject bmp = decode_file(env, v);
        if (bmp != nullptr && set_bmp != nullptr) {
            env->CallVoidMethod(native_, set_bmp, bmp);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        if (bmp != nullptr) env->DeleteLocalRef(bmp);
    }
    env->DeleteLocalRef(cls);
}

void image_button_handler<platform::android>::apply_aspect(aspect_mode v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ImageView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jclass st_cls = env->FindClass("android/widget/ImageView$ScaleType");
    if (st_cls == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return; }
    const char* name = "FIT_CENTER";
    switch (v) {
        case aspect_mode::aspect_fit:  name = "FIT_CENTER";  break;
        case aspect_mode::aspect_fill: name = "CENTER_CROP"; break;
        case aspect_mode::fill:        name = "FIT_XY";      break;
        case aspect_mode::center:      name = "CENTER";      break;
    }
    jfieldID fid = env->GetStaticFieldID(st_cls, name, "Landroid/widget/ImageView$ScaleType;");
    if (fid != nullptr) {
        jobject st = env->GetStaticObjectField(st_cls, fid);
        jmethodID set_st = env->GetMethodID(cls, "setScaleType", "(Landroid/widget/ImageView$ScaleType;)V");
        if (set_st != nullptr && st != nullptr) {
            env->CallVoidMethod(native_, set_st, st);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        if (st != nullptr) env->DeleteLocalRef(st);
    }
    env->DeleteLocalRef(st_cls);
    env->DeleteLocalRef(cls);
}

void image_button_handler<platform::android>::map_source(image_button& b) {
    apply_source(b.source.get());
    b.source.changed.subscribe(source_slot_, source_cb_);
}
void image_button_handler<platform::android>::map_aspect(image_button& b) {
    apply_aspect(b.aspect.get());
    b.aspect.changed.subscribe(aspect_slot_, aspect_cb_);
}

} // namespace mpapp

#endif // __ANDROID__
