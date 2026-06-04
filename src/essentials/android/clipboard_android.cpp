// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Android clipboard backend implementation. Confines all JNI usage to this
// translation unit; the header (clipboard_android.hpp) stays JNI-free.

#include "mpapp/essentials/android/clipboard_android.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

namespace {

// Obtain the ClipboardManager via Context.getSystemService(CLIPBOARD_SERVICE).
// Returns a local ref the caller must DeleteLocalRef, or nullptr on failure.
jobject get_clipboard_manager(JNIEnv* env, jobject context) {
    if (context == nullptr) return nullptr;
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass ctx_cls = env->FindClass("android/content/Context");
    if (ctx_cls == nullptr) { env->ExceptionClear(); return nullptr; }

    // Context.CLIPBOARD_SERVICE is the constant string "clipboard".
    jmethodID get_service = env->GetMethodID(
        ctx_cls, "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;");
    if (get_service == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(ctx_cls);
        return nullptr;
    }

    jstring service_name = env->NewStringUTF("clipboard");
    jobject manager = env->CallObjectMethod(context, get_service, service_name);
    if (env->ExceptionCheck()) { env->ExceptionClear(); manager = nullptr; }
    env->DeleteLocalRef(service_name);
    env->DeleteLocalRef(ctx_cls);
    return manager;
}

} // namespace

void android_clipboard::set_text(const std::string& text) {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) {
        clipboard_content_changed.emit(!text.empty());
        return;
    }
    jobject context = detail::get_activity();
    jobject manager = get_clipboard_manager(env, context);
    if (manager == nullptr) {
        clipboard_content_changed.emit(!text.empty());
        return;
    }

    jclass clipdata_cls = env->FindClass("android/content/ClipData");
    if (clipdata_cls == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(manager);
        clipboard_content_changed.emit(!text.empty());
        return;
    }

    // ClipData.newPlainText(CharSequence label, CharSequence text).
    jmethodID new_plain = env->GetStaticMethodID(
        clipdata_cls, "newPlainText",
        "(Ljava/lang/CharSequence;Ljava/lang/CharSequence;)"
        "Landroid/content/ClipData;");
    if (new_plain == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(clipdata_cls);
        env->DeleteLocalRef(manager);
        clipboard_content_changed.emit(!text.empty());
        return;
    }

    jstring label = env->NewStringUTF("mpapp");
    jstring value = env->NewStringUTF(text.c_str());
    jobject clip   = env->CallStaticObjectMethod(
        clipdata_cls, new_plain, label, value);
    if (env->ExceptionCheck()) { env->ExceptionClear(); clip = nullptr; }

    if (clip != nullptr) {
        jclass mgr_cls = env->GetObjectClass(manager);
        if (mgr_cls != nullptr) {
            jmethodID set_primary = env->GetMethodID(
                mgr_cls, "setPrimaryClip",
                "(Landroid/content/ClipData;)V");
            if (set_primary != nullptr) {
                env->CallVoidMethod(manager, set_primary, clip);
                if (env->ExceptionCheck()) env->ExceptionClear();
            }
            env->DeleteLocalRef(mgr_cls);
        }
        env->DeleteLocalRef(clip);
    }

    env->DeleteLocalRef(value);
    env->DeleteLocalRef(label);
    env->DeleteLocalRef(clipdata_cls);
    env->DeleteLocalRef(manager);

    clipboard_content_changed.emit(!text.empty());
}

std::optional<std::string> android_clipboard::get_text() const {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return std::nullopt;
    jobject context = detail::get_activity();
    jobject manager = get_clipboard_manager(env, context);
    if (manager == nullptr) return std::nullopt;

    std::optional<std::string> result = std::nullopt;

    jclass mgr_cls = env->GetObjectClass(manager);
    if (mgr_cls == nullptr) {
        env->DeleteLocalRef(manager);
        return std::nullopt;
    }

    jmethodID get_primary = env->GetMethodID(
        mgr_cls, "getPrimaryClip", "()Landroid/content/ClipData;");
    if (get_primary == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(mgr_cls);
        env->DeleteLocalRef(manager);
        return std::nullopt;
    }

    jobject clip = env->CallObjectMethod(manager, get_primary);
    if (env->ExceptionCheck()) { env->ExceptionClear(); clip = nullptr; }

    if (clip != nullptr) {
        jclass clip_cls = env->GetObjectClass(clip);
        if (clip_cls != nullptr) {
            jmethodID get_count = env->GetMethodID(clip_cls, "getItemCount", "()I");
            jmethodID get_item  = env->GetMethodID(
                clip_cls, "getItemAt", "(I)Landroid/content/ClipData$Item;");
            jint count = 0;
            if (get_count != nullptr) {
                count = env->CallIntMethod(clip, get_count);
                if (env->ExceptionCheck()) { env->ExceptionClear(); count = 0; }
            }

            if (count > 0 && get_item != nullptr) {
                jobject item = env->CallObjectMethod(clip, get_item, 0);
                if (env->ExceptionCheck()) { env->ExceptionClear(); item = nullptr; }
                if (item != nullptr) {
                    jclass item_cls = env->GetObjectClass(item);
                    if (item_cls != nullptr) {
                        // ClipData.Item.getText() -> CharSequence.
                        jmethodID get_text_m = env->GetMethodID(
                            item_cls, "getText", "()Ljava/lang/CharSequence;");
                        if (get_text_m != nullptr) {
                            jobject cs = env->CallObjectMethod(item, get_text_m);
                            if (env->ExceptionCheck()) { env->ExceptionClear(); cs = nullptr; }
                            if (cs != nullptr) {
                                jclass cs_cls = env->GetObjectClass(cs);
                                if (cs_cls != nullptr) {
                                    jmethodID to_string = env->GetMethodID(
                                        cs_cls, "toString", "()Ljava/lang/String;");
                                    if (to_string != nullptr) {
                                        jobject sobj = env->CallObjectMethod(cs, to_string);
                                        if (env->ExceptionCheck()) {
                                            env->ExceptionClear();
                                            sobj = nullptr;
                                        }
                                        if (sobj != nullptr) {
                                            auto jstr = static_cast<jstring>(sobj);
                                            const char* utf8 =
                                                env->GetStringUTFChars(jstr, nullptr);
                                            if (utf8 != nullptr) {
                                                result = std::string(utf8);
                                                env->ReleaseStringUTFChars(jstr, utf8);
                                            }
                                            env->DeleteLocalRef(sobj);
                                        }
                                    }
                                    env->DeleteLocalRef(cs_cls);
                                }
                                env->DeleteLocalRef(cs);
                            }
                        }
                        env->DeleteLocalRef(item_cls);
                    }
                    env->DeleteLocalRef(item);
                }
            }
            env->DeleteLocalRef(clip_cls);
        }
        env->DeleteLocalRef(clip);
    }

    env->DeleteLocalRef(mgr_cls);
    env->DeleteLocalRef(manager);
    return result;
}

bool android_clipboard::has_text() const {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return false;
    jobject context = detail::get_activity();
    jobject manager = get_clipboard_manager(env, context);
    if (manager == nullptr) return false;

    bool result = false;

    jclass mgr_cls = env->GetObjectClass(manager);
    if (mgr_cls == nullptr) {
        env->DeleteLocalRef(manager);
        return false;
    }

    jmethodID has_primary = env->GetMethodID(mgr_cls, "hasPrimaryClip", "()Z");
    if (has_primary == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(mgr_cls);
        env->DeleteLocalRef(manager);
        return false;
    }

    jboolean has = env->CallBooleanMethod(manager, has_primary);
    if (env->ExceptionCheck()) { env->ExceptionClear(); has = JNI_FALSE; }

    if (has == JNI_TRUE) {
        jmethodID get_desc = env->GetMethodID(
            mgr_cls, "getPrimaryClipDescription",
            "()Landroid/content/ClipDescription;");
        if (get_desc != nullptr) {
            jobject desc = env->CallObjectMethod(manager, get_desc);
            if (env->ExceptionCheck()) { env->ExceptionClear(); desc = nullptr; }
            if (desc != nullptr) {
                jclass desc_cls = env->GetObjectClass(desc);
                if (desc_cls != nullptr) {
                    jmethodID has_mime = env->GetMethodID(
                        desc_cls, "hasMimeType", "(Ljava/lang/String;)Z");
                    if (has_mime != nullptr) {
                        // ClipDescription.MIMETYPE_TEXT_PLAIN / _HTML.
                        jstring plain = env->NewStringUTF("text/plain");
                        jboolean is_plain =
                            env->CallBooleanMethod(desc, has_mime, plain);
                        if (env->ExceptionCheck()) { env->ExceptionClear(); is_plain = JNI_FALSE; }
                        env->DeleteLocalRef(plain);

                        jboolean is_html = JNI_FALSE;
                        if (is_plain != JNI_TRUE) {
                            jstring html = env->NewStringUTF("text/html");
                            is_html = env->CallBooleanMethod(desc, has_mime, html);
                            if (env->ExceptionCheck()) { env->ExceptionClear(); is_html = JNI_FALSE; }
                            env->DeleteLocalRef(html);
                        }
                        result = (is_plain == JNI_TRUE) || (is_html == JNI_TRUE);
                    }
                    env->DeleteLocalRef(desc_cls);
                }
                env->DeleteLocalRef(desc);
            }
        }
    }

    env->DeleteLocalRef(mgr_cls);
    env->DeleteLocalRef(manager);
    return result;
}

} // namespace mpapp

#endif // __ANDROID__
