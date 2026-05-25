// SPDX-License-Identifier: Apache-2.0
// Android basic_entry_cell handler — horizontal LinearLayout: leading
// TextView (basic_label) + trailing EditText (text, weight=1). TextWatcher
// (kind=3) echoes user input back into `text`. EditorActionListener
// (kind=1) emits `completed` on IME Done/Go/Send/Next/Search. Keyboard
// kind maps to TextView InputType bits.

#ifndef MPAPP_HANDLERS_ANDROID_ENTRY_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_ENTRY_CELL_HANDLER_HPP

#include <string>

#include "../../internal/basic_entry_cell.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class entry_cell_handler<platform::android> {
public:
    entry_cell_handler();
    ~entry_cell_handler();

    entry_cell_handler(const entry_cell_handler&)            = delete;
    entry_cell_handler& operator=(const entry_cell_handler&) = delete;
    entry_cell_handler(entry_cell_handler&&)                 = delete;
    entry_cell_handler& operator=(entry_cell_handler&&)      = delete;

    void map_label(basic_entry_cell& c);
    void map_text(basic_entry_cell& c);
    void map_placeholder(basic_entry_cell& c);
    void map_keyboard(basic_entry_cell& c);

    jobject native() const noexcept { return native_; }

    // Called from text_watcher_dispatch (kind=3).
    void on_native_text_changed(const std::string& text);
    // Called from editor_action_dispatch (kind=1).
    void on_native_editor_action(int action_id);

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_entry_cell& /*x*/) noexcept {}


private:
    void apply_label(const std::string& v);
    void apply_text(const std::string& v);
    void apply_placeholder(const std::string& v);
    void apply_keyboard(keyboard_kind v);

    struct label_cb_t {
        entry_cell_handler<platform::android>* self;
        void operator()(const std::string& v) const { self->apply_label(v); }
    };
    struct text_cb_t {
        entry_cell_handler<platform::android>* self;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct placeholder_cb_t {
        entry_cell_handler<platform::android>* self;
        void operator()(const std::string& v) const { self->apply_placeholder(v); }
    };
    struct keyboard_cb_t {
        entry_cell_handler<platform::android>* self;
        void operator()(keyboard_kind v) const { self->apply_keyboard(v); }
    };

    jobject native_       = nullptr;  // LinearLayout horizontal
    jobject label_view_   = nullptr;  // TextView (basic_label)
    jobject edit_text_    = nullptr;  // EditText
    jobject watcher_      = nullptr;  // MppTextWatcher
    jobject ime_listener_ = nullptr;  // MppEditorActionListener
    bool    suppress_echo_ = false;
    basic_entry_cell* bound_     = nullptr;

    label_cb_t                          label_cb_{this};
    text_cb_t                           text_cb_{this};
    placeholder_cb_t                    placeholder_cb_{this};
    keyboard_cb_t                       keyboard_cb_{this};
    signal_slot<const std::string&>     label_slot_{};
    signal_slot<const std::string&>     text_slot_{};
    signal_slot<const std::string&>     placeholder_slot_{};
    signal_slot<const keyboard_kind&>   keyboard_slot_{};
};

void android_entry_cell_dispatch_text_changed(entry_cell_handler<platform::android>* h,
                                              const std::string& text);
void android_entry_cell_dispatch_editor_action(entry_cell_handler<platform::android>* h,
                                               int action_id);

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_ENTRY_CELL_HANDLER_HPP
