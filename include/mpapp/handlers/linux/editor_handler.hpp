// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_editor handler — wraps GtkTextView + GtkTextBuffer.
// The native widget itself is the GtkTextView; the buffer is accessed
// via gtk_text_view_get_buffer for both reads and writes.

#ifndef MPAPP_HANDLERS_LINUX_EDITOR_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_EDITOR_HANDLER_HPP

#include "../../internal/basic_editor.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <string>

namespace mpapp::internal {

template <>
class editor_handler<platform::linux_> {
public:
    editor_handler();
    ~editor_handler();

    editor_handler(const editor_handler&)            = delete;
    editor_handler& operator=(const editor_handler&) = delete;
    editor_handler(editor_handler&&)                 = delete;
    editor_handler& operator=(editor_handler&&)      = delete;

    void map_text(basic_editor& e);
    void map_is_read_only(basic_editor& e);
    // GtkTextView has no placeholder concept; map_placeholder is a no-op
    // for the Linux handler (cross-platform parity preserved by exposing
    // the function — the GTK4 widget shows the empty buffer as blank).
    void map_placeholder(basic_editor&) noexcept {}

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_text(const std::string& text);
    void apply_is_read_only(bool ro);

    struct text_cb_t {
        editor_handler<platform::linux_>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct readonly_cb_t {
        editor_handler<platform::linux_>* self = nullptr;
        void operator()(bool v) const { self->apply_is_read_only(v); }
    };

    void*                            native_              = nullptr;  // GtkTextView*
    unsigned long                    changed_handler_id_  = 0;
    bool                             suppress_echo_       = false;
    text_cb_t                        text_cb_{this};
    readonly_cb_t                    readonly_cb_{this};
    signal_slot<const std::string&>  text_slot_{};
    signal_slot<const bool&>         readonly_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_EDITOR_HANDLER_HPP
