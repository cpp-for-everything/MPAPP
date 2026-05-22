// SPDX-License-Identifier: Apache-2.0
// GTK4 switch_cell handler — horizontal GtkBox: label on the left
// (hexpand=TRUE), GtkSwitch on the right. state-set signal echoes
// user flips back into the cell's `on` Observable.

#ifndef MPAPP_HANDLERS_LINUX_SWITCH_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_SWITCH_CELL_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../switch_cell.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class switch_cell_handler<platform::linux_> {
public:
    switch_cell_handler();
    ~switch_cell_handler();

    switch_cell_handler(const switch_cell_handler&)            = delete;
    switch_cell_handler& operator=(const switch_cell_handler&) = delete;
    switch_cell_handler(switch_cell_handler&&)                 = delete;
    switch_cell_handler& operator=(switch_cell_handler&&)      = delete;

    void map_text(switch_cell& c);
    void map_on(switch_cell& c);

    void*       native() noexcept       { return native_; }   // GtkBox*
    const void* native() const noexcept { return native_; }

private:
    void apply_text(const std::string& v);
    void apply_on(bool v);

    struct text_cb_t {
        switch_cell_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct on_cb_t {
        switch_cell_handler<platform::linux_>* self;
        void operator()(bool v) const { self->apply_on(v); }
    };

    void*         native_     = nullptr;  // GtkBox*  (horizontal)
    void*         label_      = nullptr;  // GtkLabel*
    void*         switch_w_   = nullptr;  // GtkSwitch*
    unsigned long state_set_handler_id_ = 0;
    bool          suppress_echo_ = false;
    switch_cell*  bound_         = nullptr;

    text_cb_t                       text_cb_{this};
    on_cb_t                         on_cb_{this};
    signal_slot<const std::string&> text_slot_{};
    signal_slot<const bool&>        on_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_SWITCH_CELL_HANDLER_HPP
