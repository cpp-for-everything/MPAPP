// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 follow-up — Android entry handler.
//
// Wraps `android.widget.EditText`. Reverse binding (user typing into
// the EditText updates `entry::text`) is wired via a Java
// `android.text.TextWatcher` shim — see
// `examples/android_hello/.../io/mpapp/MppTextWatcher.java`.

#ifndef MPAPP_HANDLERS_ANDROID_ENTRY_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_ENTRY_HANDLER_HPP

#include "../../entry.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>
#include <string>

namespace mpapp {

template <>
class entry_handler<platform::android> {
public:
    entry_handler();
    ~entry_handler();

    entry_handler(const entry_handler&)            = delete;
    entry_handler& operator=(const entry_handler&) = delete;
    entry_handler(entry_handler&&)                 = delete;
    entry_handler& operator=(entry_handler&&)      = delete;

    void map_text(entry& e);
    void map_placeholder(entry& e);
    void map_is_read_only(entry& e);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

    // Internal — invoked by the Java-side text watcher to mirror
    // user-driven edits back into the cross-platform Observable.
    void on_native_text_changed(const std::string& text);

private:
    void apply_text(const std::string& text);
    void apply_placeholder(const std::string& text);
    void apply_is_read_only(bool ro);

    struct text_callback {
        entry_handler<platform::android>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct placeholder_callback {
        entry_handler<platform::android>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_placeholder(v); }
    };
    struct readonly_callback {
        entry_handler<platform::android>* self = nullptr;
        void operator()(bool v) const { self->apply_is_read_only(v); }
    };

    jobject                          native_   = nullptr;  // global ref to EditText
    jobject                          watcher_  = nullptr;  // global ref to MppTextWatcher
    entry*                           bound_    = nullptr;
    bool                             suppress_echo_ = false;
    text_callback                    text_cb_{this};
    placeholder_callback             placeholder_cb_{this};
    readonly_callback                readonly_cb_{this};
    signal_slot<const std::string&>  text_slot_{};
    signal_slot<const std::string&>  placeholder_slot_{};
    signal_slot<const bool&>         readonly_slot_{};
};

// Trampoline for the Java MppTextWatcher to call back into the
// handler when the EditText's text changes.
void android_entry_dispatch_text_changed(entry_handler<platform::android>* h,
                                         const std::string& text);

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_ENTRY_HANDLER_HPP
