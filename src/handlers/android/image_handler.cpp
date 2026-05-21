// SPDX-License-Identifier: Apache-2.0
// Android image handler implementation.

#include "mpapp/handlers/android/image_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

namespace {

jobject make_image_view(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ImageView");
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

// BitmapFactory.decodeFile(String).
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

} // namespace

image_handler<platform::android>::image_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_image_view(env, detail::get_activity());
}

image_handler<platform::android>::~image_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void image_handler<platform::android>::apply_source(const std::string& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ImageView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    if (v.empty()) {
        jmethodID clear_m = env->GetMethodID(cls, "setImageBitmap", "(Landroid/graphics/Bitmap;)V");
        if (clear_m != nullptr) {
            env->CallVoidMethod(native_, clear_m, nullptr);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
    } else {
        jobject bmp = decode_file(env, v);
        if (bmp != nullptr) {
            jmethodID set_bmp = env->GetMethodID(cls, "setImageBitmap", "(Landroid/graphics/Bitmap;)V");
            if (set_bmp != nullptr) {
                env->CallVoidMethod(native_, set_bmp, bmp);
                if (env->ExceptionCheck()) env->ExceptionClear();
            }
            env->DeleteLocalRef(bmp);
        }
    }
    env->DeleteLocalRef(cls);
}

void image_handler<platform::android>::apply_aspect(aspect_mode v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ImageView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jclass st_cls = env->FindClass("android/widget/ImageView$ScaleType");
    if (st_cls == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return; }
    // ScaleType.FIT_CENTER, CENTER_CROP, FIT_XY, CENTER (static fields).
    const char* field_name = "FIT_CENTER";
    switch (v) {
        case aspect_mode::aspect_fit:  field_name = "FIT_CENTER";  break;
        case aspect_mode::aspect_fill: field_name = "CENTER_CROP"; break;
        case aspect_mode::fill:        field_name = "FIT_XY";      break;
        case aspect_mode::center:      field_name = "CENTER";      break;
    }
    jfieldID fid = env->GetStaticFieldID(st_cls, field_name, "Landroid/widget/ImageView$ScaleType;");
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

void image_handler<platform::android>::map_source(image& i) {
    apply_source(i.source.get());
    i.source.changed.subscribe(source_slot_, source_cb_);
}
void image_handler<platform::android>::map_aspect(image& i) {
    apply_aspect(i.aspect.get());
    i.aspect.changed.subscribe(aspect_slot_, aspect_cb_);
}

} // namespace mpapp


// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register image so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/image.hpp"

namespace {

jobject dispatch_image(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::image*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_image); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
