// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Android share backend implementation. Confines all JNI usage to this
// translation unit; the header (share_android.hpp) stays JNI-free.

#include "mpapp/essentials/android/share_android.hpp"

#if defined(__ANDROID__)

#include <jni.h>

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

namespace {

// ---------------------------------------------------------------------------
// JNI string helpers
// ---------------------------------------------------------------------------

// Create a jstring from a UTF-8 std::string. Returns nullptr on failure.
// Caller is responsible for DeleteLocalRef.
jstring to_jstring(JNIEnv* env, const std::string& s) {
    return env->NewStringUTF(s.c_str());
}

// ---------------------------------------------------------------------------
// Intent construction helpers
// ---------------------------------------------------------------------------

// Resolve android.content.Intent class. Returns local ref or nullptr.
jclass find_intent_class(JNIEnv* env) {
    jclass cls = env->FindClass("android/content/Intent");
    if (cls == nullptr) { env->ExceptionClear(); }
    return cls;
}

// Get a static String field from a class (e.g. Intent.ACTION_SEND).
// Returns a local ref jstring or nullptr.
jstring get_static_string_field(JNIEnv* env, jclass cls, const char* field_name) {
    jfieldID fid = env->GetStaticFieldID(cls, field_name, "Ljava/lang/String;");
    if (fid == nullptr) { env->ExceptionClear(); return nullptr; }
    auto obj = static_cast<jstring>(env->GetStaticObjectField(cls, fid));
    if (env->ExceptionCheck()) { env->ExceptionClear(); return nullptr; }
    return obj;
}

// new Intent(action) — returns local ref or nullptr.
jobject new_intent(JNIEnv* env, jclass intent_cls, jstring action) {
    jmethodID ctor = env->GetMethodID(
        intent_cls, "<init>", "(Ljava/lang/String;)V");
    if (ctor == nullptr) { env->ExceptionClear(); return nullptr; }
    jobject intent = env->NewObject(intent_cls, ctor, action);
    if (env->ExceptionCheck()) { env->ExceptionClear(); return nullptr; }
    return intent;
}

// intent.setType(mime) — returns false on failure.
bool intent_set_type(JNIEnv* env, jclass intent_cls,
                     jobject intent, const std::string& mime) {
    jmethodID mid = env->GetMethodID(
        intent_cls, "setType", "(Ljava/lang/String;)Landroid/content/Intent;");
    if (mid == nullptr) { env->ExceptionClear(); return false; }
    jstring jmime = env->NewStringUTF(mime.c_str());
    if (jmime == nullptr) return false;
    jobject ret = env->CallObjectMethod(intent, mid, jmime);
    if (env->ExceptionCheck()) { env->ExceptionClear(); }
    if (ret != nullptr) env->DeleteLocalRef(ret);
    env->DeleteLocalRef(jmime);
    return true;
}

// intent.putExtra(key, value) — String variant.
// key_field_name is the Intent.EXTRA_* static field name on intent_cls.
bool intent_put_extra_string(JNIEnv* env, jclass intent_cls,
                             jobject intent,
                             const char* key_field_name,
                             const std::string& value) {
    jstring key = get_static_string_field(env, intent_cls, key_field_name);
    if (key == nullptr) return false;
    jstring jval = env->NewStringUTF(value.c_str());
    if (jval == nullptr) {
        env->DeleteLocalRef(key);
        return false;
    }
    jmethodID mid = env->GetMethodID(
        intent_cls, "putExtra",
        "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;");
    if (mid == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(jval);
        env->DeleteLocalRef(key);
        return false;
    }
    jobject ret = env->CallObjectMethod(intent, mid, key, jval);
    if (env->ExceptionCheck()) { env->ExceptionClear(); }
    if (ret != nullptr) env->DeleteLocalRef(ret);
    env->DeleteLocalRef(jval);
    env->DeleteLocalRef(key);
    return true;
}

// intent.addFlags(int flags). Returns false on failure.
bool intent_add_flags(JNIEnv* env, jclass intent_cls,
                      jobject intent, jint flags) {
    jmethodID mid = env->GetMethodID(
        intent_cls, "addFlags", "(I)Landroid/content/Intent;");
    if (mid == nullptr) { env->ExceptionClear(); return false; }
    jobject ret = env->CallObjectMethod(intent, mid, flags);
    if (env->ExceptionCheck()) { env->ExceptionClear(); }
    if (ret != nullptr) env->DeleteLocalRef(ret);
    return true;
}

// Intent.createChooser(target, title) — static method.
// Returns local ref jobject or nullptr.
jobject create_chooser(JNIEnv* env, jclass intent_cls,
                       jobject target_intent, const std::string& title) {
    jmethodID mid = env->GetStaticMethodID(
        intent_cls, "createChooser",
        "(Landroid/content/Intent;Ljava/lang/CharSequence;)"
        "Landroid/content/Intent;");
    if (mid == nullptr) { env->ExceptionClear(); return nullptr; }
    jstring jtitle = env->NewStringUTF(title.c_str());
    if (jtitle == nullptr) return nullptr;
    jobject chooser = env->CallStaticObjectMethod(
        intent_cls, mid, target_intent, jtitle);
    if (env->ExceptionCheck()) { env->ExceptionClear(); chooser = nullptr; }
    env->DeleteLocalRef(jtitle);
    return chooser;
}

// context.startActivity(intent). Returns false on failure.
bool start_activity(JNIEnv* env, jobject context, jobject intent) {
    jclass ctx_cls = env->FindClass("android/content/Context");
    if (ctx_cls == nullptr) { env->ExceptionClear(); return false; }
    jmethodID mid = env->GetMethodID(
        ctx_cls, "startActivity", "(Landroid/content/Intent;)V");
    env->DeleteLocalRef(ctx_cls);
    if (mid == nullptr) { env->ExceptionClear(); return false; }
    env->CallVoidMethod(context, mid, intent);
    if (env->ExceptionCheck()) { env->ExceptionClear(); return false; }
    return true;
}

// Build a android.net.Uri from a file path via Uri.fromFile(File).
// Returns local ref or nullptr.
jobject uri_from_path(JNIEnv* env, const std::string& path) {
    // new java.io.File(path)
    jclass file_cls = env->FindClass("java/io/File");
    if (file_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID file_ctor = env->GetMethodID(
        file_cls, "<init>", "(Ljava/lang/String;)V");
    if (file_ctor == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(file_cls);
        return nullptr;
    }
    jstring jpath = env->NewStringUTF(path.c_str());
    if (jpath == nullptr) {
        env->DeleteLocalRef(file_cls);
        return nullptr;
    }
    jobject file_obj = env->NewObject(file_cls, file_ctor, jpath);
    env->DeleteLocalRef(jpath);
    env->DeleteLocalRef(file_cls);
    if (env->ExceptionCheck() || file_obj == nullptr) {
        env->ExceptionClear();
        if (file_obj != nullptr) env->DeleteLocalRef(file_obj);
        return nullptr;
    }

    // Uri.fromFile(file_obj)
    jclass uri_cls = env->FindClass("android/net/Uri");
    if (uri_cls == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(file_obj); return nullptr; }
    jmethodID from_file = env->GetStaticMethodID(
        uri_cls, "fromFile", "(Ljava/io/File;)Landroid/net/Uri;");
    if (from_file == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(uri_cls);
        env->DeleteLocalRef(file_obj);
        return nullptr;
    }
    jobject uri = env->CallStaticObjectMethod(uri_cls, from_file, file_obj);
    if (env->ExceptionCheck()) { env->ExceptionClear(); uri = nullptr; }
    env->DeleteLocalRef(uri_cls);
    env->DeleteLocalRef(file_obj);
    return uri;
}

// Determine whether the context object is an Activity.
// We check via instanceof (IsInstanceOf) on android.app.Activity.
bool is_activity_context(JNIEnv* env, jobject context) {
    if (context == nullptr) return false;
    jclass activity_cls = env->FindClass("android/app/Activity");
    if (activity_cls == nullptr) { env->ExceptionClear(); return false; }
    jboolean result = env->IsInstanceOf(context, activity_cls);
    env->DeleteLocalRef(activity_cls);
    return result == JNI_TRUE;
}

// FLAG_ACTIVITY_NEW_TASK = 0x10000000
constexpr jint kFlagActivityNewTask = 0x10000000;

} // namespace

// ---------------------------------------------------------------------------
// android_share::request(share_text_request)
// ---------------------------------------------------------------------------

void android_share::request(const share_text_request& req) {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    jobject context = detail::get_activity();
    if (context == nullptr) return;

    jclass intent_cls = find_intent_class(env);
    if (intent_cls == nullptr) return;

    // Build the share text to use: text, optionally followed by the URI.
    std::string share_text = req.text;
    if (!req.uri.empty()) {
        if (!share_text.empty()) share_text += '\n';
        share_text += req.uri;
    }

    // Intent(ACTION_SEND)
    jstring action_send = get_static_string_field(env, intent_cls, "ACTION_SEND");
    if (action_send == nullptr) {
        env->DeleteLocalRef(intent_cls);
        return;
    }

    jobject intent = new_intent(env, intent_cls, action_send);
    env->DeleteLocalRef(action_send);
    if (intent == nullptr) {
        env->DeleteLocalRef(intent_cls);
        return;
    }

    // setType("text/plain")
    intent_set_type(env, intent_cls, intent, "text/plain");

    // putExtra(EXTRA_TEXT, share_text)
    if (!share_text.empty()) {
        intent_put_extra_string(env, intent_cls, intent, "EXTRA_TEXT", share_text);
    }

    // putExtra(EXTRA_SUBJECT, subject) when non-empty
    if (!req.subject.empty()) {
        intent_put_extra_string(env, intent_cls, intent, "EXTRA_SUBJECT", req.subject);
    }

    // FLAG_ACTIVITY_NEW_TASK when context is not an Activity
    if (!is_activity_context(env, context)) {
        intent_add_flags(env, intent_cls, intent, kFlagActivityNewTask);
    }

    // createChooser
    jobject chooser = create_chooser(env, intent_cls, intent, req.title);
    if (chooser == nullptr) {
        // Fall back to launching the bare intent
        chooser = intent;
        intent  = nullptr;
    }

    // Add NEW_TASK to the chooser wrapper too when needed
    if (!is_activity_context(env, context)) {
        intent_add_flags(env, intent_cls, chooser, kFlagActivityNewTask);
    }

    start_activity(env, context, chooser);

    env->DeleteLocalRef(chooser);
    if (intent != nullptr) env->DeleteLocalRef(intent);
    env->DeleteLocalRef(intent_cls);
}

// ---------------------------------------------------------------------------
// android_share::request(share_file_request)
// ---------------------------------------------------------------------------

void android_share::request(const share_file_request& req) {
    if (req.files.empty()) return;

    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    jobject context = detail::get_activity();
    if (context == nullptr) return;

    jclass intent_cls = find_intent_class(env);
    if (intent_cls == nullptr) return;

    const bool multiple = req.files.size() > 1;

    // Determine action string field name
    const char* action_field = multiple ? "ACTION_SEND_MULTIPLE" : "ACTION_SEND";
    jstring action = get_static_string_field(env, intent_cls, action_field);
    if (action == nullptr) {
        env->DeleteLocalRef(intent_cls);
        return;
    }

    jobject intent = new_intent(env, intent_cls, action);
    env->DeleteLocalRef(action);
    if (intent == nullptr) {
        env->DeleteLocalRef(intent_cls);
        return;
    }

    // Choose MIME type: common type if all files share one, else "*/*"
    const std::string& first_mime = req.files[0].content_type;
    std::string mime_type = first_mime.empty() ? "application/octet-stream" : first_mime;
    for (std::size_t i = 1; i < req.files.size(); ++i) {
        const std::string& m = req.files[i].content_type;
        if (m != mime_type) { mime_type = "*/*"; break; }
    }
    intent_set_type(env, intent_cls, intent, mime_type);

    if (!multiple) {
        // Single file: putExtra(EXTRA_STREAM, uri)
        jobject uri = uri_from_path(env, req.files[0].path);
        if (uri != nullptr) {
            jstring key = get_static_string_field(env, intent_cls, "EXTRA_STREAM");
            if (key != nullptr) {
                jmethodID mid = env->GetMethodID(
                    intent_cls, "putExtra",
                    "(Ljava/lang/String;Landroid/os/Parcelable;)"
                    "Landroid/content/Intent;");
                if (mid == nullptr) {
                    env->ExceptionClear();
                    // Try the Serializable overload — Uri implements Parcelable,
                    // but fall through; the intent may still work via chooser.
                } else {
                    jobject ret = env->CallObjectMethod(intent, mid, key, uri);
                    if (env->ExceptionCheck()) { env->ExceptionClear(); }
                    if (ret != nullptr) env->DeleteLocalRef(ret);
                }
                env->DeleteLocalRef(key);
            }
            env->DeleteLocalRef(uri);
        }
    } else {
        // Multiple files: putParcelableArrayListExtra(EXTRA_STREAM, list)
        // Build an ArrayList<Uri>
        jclass arraylist_cls = env->FindClass("java/util/ArrayList");
        if (arraylist_cls == nullptr) {
            env->ExceptionClear();
            env->DeleteLocalRef(intent);
            env->DeleteLocalRef(intent_cls);
            return;
        }
        jmethodID al_ctor = env->GetMethodID(arraylist_cls, "<init>", "(I)V");
        if (al_ctor == nullptr) {
            env->ExceptionClear();
            env->DeleteLocalRef(arraylist_cls);
            env->DeleteLocalRef(intent);
            env->DeleteLocalRef(intent_cls);
            return;
        }
        jobject uri_list = env->NewObject(
            arraylist_cls, al_ctor,
            static_cast<jint>(req.files.size()));
        if (uri_list == nullptr || env->ExceptionCheck()) {
            env->ExceptionClear();
            if (uri_list != nullptr) env->DeleteLocalRef(uri_list);
            env->DeleteLocalRef(arraylist_cls);
            env->DeleteLocalRef(intent);
            env->DeleteLocalRef(intent_cls);
            return;
        }

        jmethodID al_add = env->GetMethodID(
            arraylist_cls, "add", "(Ljava/lang/Object;)Z");
        if (al_add == nullptr) { env->ExceptionClear(); }

        for (const auto& sf : req.files) {
            jobject uri = uri_from_path(env, sf.path);
            if (uri != nullptr) {
                if (al_add != nullptr) {
                    env->CallBooleanMethod(uri_list, al_add, uri);
                    if (env->ExceptionCheck()) env->ExceptionClear();
                }
                env->DeleteLocalRef(uri);
            }
        }
        env->DeleteLocalRef(arraylist_cls);

        // intent.putParcelableArrayListExtra(EXTRA_STREAM, uri_list)
        jstring key = get_static_string_field(env, intent_cls, "EXTRA_STREAM");
        if (key != nullptr) {
            jmethodID mid = env->GetMethodID(
                intent_cls, "putParcelableArrayListExtra",
                "(Ljava/lang/String;Ljava/util/ArrayList;)"
                "Landroid/content/Intent;");
            if (mid == nullptr) { env->ExceptionClear(); }
            else {
                jobject ret = env->CallObjectMethod(intent, mid, key, uri_list);
                if (env->ExceptionCheck()) { env->ExceptionClear(); }
                if (ret != nullptr) env->DeleteLocalRef(ret);
            }
            env->DeleteLocalRef(key);
        }
        env->DeleteLocalRef(uri_list);
    }

    // FLAG_ACTIVITY_NEW_TASK when context is not an Activity
    if (!is_activity_context(env, context)) {
        intent_add_flags(env, intent_cls, intent, kFlagActivityNewTask);
    }

    // createChooser
    jobject chooser = create_chooser(env, intent_cls, intent, req.title);
    if (chooser == nullptr) {
        chooser = intent;
        intent  = nullptr;
    }

    if (!is_activity_context(env, context)) {
        intent_add_flags(env, intent_cls, chooser, kFlagActivityNewTask);
    }

    start_activity(env, context, chooser);

    env->DeleteLocalRef(chooser);
    if (intent != nullptr) env->DeleteLocalRef(intent);
    env->DeleteLocalRef(intent_cls);
}

} // namespace mpapp

#endif // __ANDROID__
