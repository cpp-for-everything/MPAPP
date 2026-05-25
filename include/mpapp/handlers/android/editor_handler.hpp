// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_editor handler — wraps android.widget.EditText
// configured for multi-line input. Reuses the io.mpapp.MppTextWatcher
// shim from Entry; the JNI trampoline routes via a kind discriminator.

#ifndef MPAPP_HANDLERS_ANDROID_EDITOR_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_EDITOR_HANDLER_HPP

#include "../../internal/basic_editor.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>
#include <string>

namespace mpapp::internal {

template <>
class editor_handler<platform::android> {
public:
    editor_handler();
    ~editor_handler();

    editor_handler(const editor_handler&)            = delete;
    editor_handler& operator=(const editor_handler&) = delete;
    editor_handler(editor_handler&&)                 = delete;
    editor_handler& operator=(editor_handler&&)      = delete;

    void map_text(basic_editor& e);
    void map_placeholder(basic_editor& e);
    void map_is_read_only(basic_editor& e);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

    void on_native_text_changed(const std::string& text);

private:
    void apply_text(const std::string& text);
    void apply_placeholder(const std::string& text);
    void apply_is_read_only(bool ro);

    struct text_cb_t {
        editor_handler<platform::android>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct placeholder_cb_t {
        editor_handler<platform::android>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_placeholder(v); }
    };
    struct readonly_cb_t {
        editor_handler<platform::android>* self = nullptr;
        void operator()(bool v) const { self->apply_is_read_only(v); }
    };

    jobject                          native_   = nullptr;  // global ref to EditText
    jobject                          watcher_  = nullptr;
    basic_editor*                          bound_    = nullptr;
    bool                             suppress_echo_ = false;
    text_cb_t                        text_cb_{this};
    placeholder_cb_t                 placeholder_cb_{this};
    readonly_cb_t                    readonly_cb_{this};
    signal_slot<const std::string&>  text_slot_{};
    signal_slot<const std::string&>  placeholder_slot_{};
    signal_slot<const bool&>         readonly_slot_{};
};

void android_editor_dispatch_text_changed(editor_handler<platform::android>* h,
                                          const std::string& text);

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_EDITOR_HANDLER_HPP
