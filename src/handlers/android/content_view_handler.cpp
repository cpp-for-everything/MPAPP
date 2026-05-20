// SPDX-License-Identifier: Apache-2.0
// Android content_view handler implementation.

#include "mpapp/handlers/android/content_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/box_view.hpp"
#include "mpapp/button.hpp"
#include "mpapp/check_box.hpp"
#include "mpapp/editor.hpp"
#include "mpapp/entry.hpp"
#include "mpapp/handlers/android/box_view_handler.hpp"
#include "mpapp/handlers/android/button_handler.hpp"
#include "mpapp/handlers/android/check_box_handler.hpp"
#include "mpapp/handlers/android/editor_handler.hpp"
#include "mpapp/handlers/android/entry_handler.hpp"
#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/label_handler.hpp"
#include "mpapp/handlers/android/radio_button_handler.hpp"
#include "mpapp/handlers/android/slider_handler.hpp"
#include "mpapp/handlers/android/stack_layout_handler.hpp"
#include "mpapp/handlers/android/switch_handler.hpp"
#include "mpapp/label.hpp"
#include "mpapp/radio_button.hpp"
#include "mpapp/slider.hpp"
#include "mpapp/stack_layout.hpp"
#include "mpapp/switch_.hpp"

namespace mpapp {

namespace {

jobject child_jobject(view* v) {
    if (auto* sl = dynamic_cast<stack_layout*>(v); sl && sl->has_handler()) return sl->handler().native();
    if (auto* b  = dynamic_cast<button*>(v);       b  && b->has_handler())  return b->handler().native();
    if (auto* l  = dynamic_cast<label*>(v);        l  && l->has_handler())  return l->handler().native();
    if (auto* e  = dynamic_cast<entry*>(v);        e  && e->has_handler())  return e->handler().native();
    if (auto* sw = dynamic_cast<switch_*>(v);      sw && sw->has_handler()) return sw->handler().native();
    if (auto* cb = dynamic_cast<check_box*>(v);    cb && cb->has_handler()) return cb->handler().native();
    if (auto* rb = dynamic_cast<radio_button*>(v); rb && rb->has_handler()) return rb->handler().native();
    if (auto* s2 = dynamic_cast<slider*>(v);       s2 && s2->has_handler()) return s2->handler().native();
    if (auto* ed = dynamic_cast<editor*>(v);       ed && ed->has_handler()) return ed->handler().native();
    if (auto* bx = dynamic_cast<box_view*>(v);     bx && bx->has_handler()) return bx->handler().native();
    return nullptr;
}

jobject make_frame_layout(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/FrameLayout");
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

} // namespace

content_view_handler<platform::android>::content_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_frame_layout(env, detail::get_activity());
}

content_view_handler<platform::android>::~content_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void content_view_handler<platform::android>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass vg = env->FindClass("android/view/ViewGroup");
    if (vg == nullptr) { env->ExceptionClear(); return; }
    jmethodID clear_m = env->GetMethodID(vg, "removeAllViews", "()V");
    if (clear_m != nullptr) {
        env->CallVoidMethod(native_, clear_m);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    jobject child = v ? child_jobject(v.get()) : nullptr;
    if (child != nullptr) {
        jmethodID add_m = env->GetMethodID(vg, "addView", "(Landroid/view/View;)V");
        if (add_m != nullptr) {
            env->CallVoidMethod(native_, add_m, child);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
    }
    env->DeleteLocalRef(vg);
}

void content_view_handler<platform::android>::map_content(content_view& c) {
    apply_content(c.content.get());
    c.content.changed.subscribe(content_slot_, content_cb_);
}

void content_view_handler<platform::android>::bind_content(content_view& c, view& child) {
    c.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp

#endif // __ANDROID__
