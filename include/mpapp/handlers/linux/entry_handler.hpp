// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 follow-up — GTK4 entry handler.

#ifndef MPAPP_HANDLERS_LINUX_ENTRY_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_ENTRY_HANDLER_HPP

#include "../../entry.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <string>

namespace mpapp {

template <>
class entry_handler<platform::linux_> {
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

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_text(const std::string& text);
    void apply_placeholder(const std::string& text);
    void apply_is_read_only(bool ro);

    struct text_callback {
        entry_handler<platform::linux_>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct placeholder_callback {
        entry_handler<platform::linux_>* self = nullptr;
        void operator()(const std::string& v) const { self->apply_placeholder(v); }
    };
    struct readonly_callback {
        entry_handler<platform::linux_>* self = nullptr;
        void operator()(bool v) const { self->apply_is_read_only(v); }
    };

    void*                            native_         = nullptr;  // GtkEntry*
    unsigned long                    changed_handler_id_ = 0;
    bool                             suppress_echo_  = false;
    text_callback                    text_cb_{this};
    placeholder_callback             placeholder_cb_{this};
    readonly_callback                readonly_cb_{this};
    signal_slot<const std::string&>  text_slot_{};
    signal_slot<const std::string&>  placeholder_slot_{};
    signal_slot<const bool&>         readonly_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_ENTRY_HANDLER_HPP
