// SPDX-License-Identifier: Apache-2.0
// GTK4 entry_cell handler — horizontal GtkBox: leading GtkLabel +
// trailing GtkEntry bound to `text`. GtkEntry "activate" signal
// (Enter key) emits `completed`. Keyboard kind maps to
// gtk_entry_set_input_purpose.

#ifndef MPAPP_HANDLERS_LINUX_ENTRY_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_ENTRY_CELL_HANDLER_HPP

#include <string>

#include "../../entry_cell.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class entry_cell_handler<platform::linux_> {
public:
    entry_cell_handler();
    ~entry_cell_handler();

    entry_cell_handler(const entry_cell_handler&)            = delete;
    entry_cell_handler& operator=(const entry_cell_handler&) = delete;
    entry_cell_handler(entry_cell_handler&&)                 = delete;
    entry_cell_handler& operator=(entry_cell_handler&&)      = delete;

    void map_label(entry_cell& c);
    void map_text(entry_cell& c);
    void map_placeholder(entry_cell& c);
    void map_keyboard(entry_cell& c);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_label(const std::string& v);
    void apply_text(const std::string& v);
    void apply_placeholder(const std::string& v);
    void apply_keyboard(keyboard_kind v);

    struct label_cb_t {
        entry_cell_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_label(v); }
    };
    struct text_cb_t {
        entry_cell_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct placeholder_cb_t {
        entry_cell_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_placeholder(v); }
    };
    struct keyboard_cb_t {
        entry_cell_handler<platform::linux_>* self;
        void operator()(keyboard_kind v) const { self->apply_keyboard(v); }
    };

    void*         native_        = nullptr;  // GtkBox* horizontal
    void*         label_w_       = nullptr;  // GtkLabel*
    void*         entry_w_       = nullptr;  // GtkEntry*
    unsigned long changed_handler_id_  = 0;
    unsigned long activate_handler_id_ = 0;
    bool          suppress_echo_ = false;
    entry_cell*   bound_         = nullptr;

    label_cb_t                          label_cb_{this};
    text_cb_t                           text_cb_{this};
    placeholder_cb_t                    placeholder_cb_{this};
    keyboard_cb_t                       keyboard_cb_{this};
    signal_slot<const std::string&>     label_slot_{};
    signal_slot<const std::string&>     text_slot_{};
    signal_slot<const std::string&>     placeholder_slot_{};
    signal_slot<const keyboard_kind&>   keyboard_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_ENTRY_CELL_HANDLER_HPP
