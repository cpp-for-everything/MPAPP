// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::android_share` — Android share-sheet backend.
// Implements `mpapp::share` using android.content.Intent.ACTION_SEND /
// ACTION_SEND_MULTIPLE, wrapped in Intent.createChooser and started via
// Context.startActivity. The Context is obtained from the JNI bridge
// (mpapp::detail::get_activity()), so no Context is required in the
// constructor. All JNI details (<jni.h>, FindClass, CallObjectMethod,
// DeleteLocalRef, AttachCurrentThread) are confined to the .cpp translation
// unit; this header stays JNI-free, mirroring the clipboard backend. No
// macros in the public API.

#ifndef MPAPP_ESSENTIALS_ANDROID_SHARE_ANDROID_HPP
#define MPAPP_ESSENTIALS_ANDROID_SHARE_ANDROID_HPP

#include "mpapp/essentials/share.hpp"

namespace mpapp {

// Android share backend. Implements `mpapp::share` by building an Android
// Intent and presenting it through the system chooser dialog.
//
// Text sharing  -> Intent(ACTION_SEND, type="text/plain") with
//                  EXTRA_TEXT and EXTRA_SUBJECT, wrapped in createChooser,
//                  started with FLAG_ACTIVITY_NEW_TASK when the stored
//                  context is the application context rather than an
//                  Activity.
//
// File sharing  -> Intent(ACTION_SEND) for a single file or
//                  Intent(ACTION_SEND_MULTIPLE) for multiple files,
//                  with EXTRA_STREAM populated from the file paths via
//                  Uri.fromFile (or a pre-built content URI if the caller
//                  has set one up), wrapped in createChooser. The MIME type
//                  is taken from share_file::content_type; "application/octet-stream"
//                  is used as a safe fallback when the field is empty.
//
// The Context is taken from detail::get_activity(), which the host
// MainActivity sets once during native init. FLAG_ACTIVITY_NEW_TASK is
// added automatically when the context is not an Activity (i.e. when
// get_activity() returns the application context).
class android_share final : public share {
public:
    android_share()  = default;
    ~android_share() = default;

    android_share(const android_share&)            = delete;
    android_share& operator=(const android_share&) = delete;
    android_share(android_share&&)                 = delete;
    android_share& operator=(android_share&&)      = delete;

    // Present the Android share chooser for plain text / URL content.
    // Maps share_text_request::text  -> Intent.EXTRA_TEXT
    //      share_text_request::subject -> Intent.EXTRA_SUBJECT
    //      share_text_request::title   -> Intent.createChooser title
    //      share_text_request::uri     -> appended to EXTRA_TEXT when non-empty
    // Silently no-ops if the JNI bridge is unavailable.
    void request(const share_text_request& req) override;

    // Present the Android share chooser for one or more files.
    // Maps share_file_request::files -> EXTRA_STREAM (Uri per file)
    //      share_file_request::title -> Intent.createChooser title
    // Uses ACTION_SEND for a single file, ACTION_SEND_MULTIPLE for many.
    // Silently no-ops if the JNI bridge is unavailable or the file list
    // is empty.
    void request(const share_file_request& req) override;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_ANDROID_SHARE_ANDROID_HPP
