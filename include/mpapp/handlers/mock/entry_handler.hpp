// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-platform specialisation of `entry_handler`.

#ifndef MPAPP_HANDLERS_MOCK_ENTRY_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_ENTRY_HANDLER_HPP

#include <string>

#include "../../internal/basic_entry.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class entry_handler<platform::mock> : public mock_handler_base {
public:
    entry_handler()  = default;
    ~entry_handler() = default;

    entry_handler(const entry_handler&)            = delete;
    entry_handler& operator=(const entry_handler&) = delete;
    entry_handler(entry_handler&&)                 = delete;
    entry_handler& operator=(entry_handler&&)      = delete;

    void map_text(basic_entry& e) {
        record_change("text", e.text.get());
        e.text.changed.subscribe(text_slot_, text_cb_);
    }

    void map_placeholder(basic_entry& e) {
        record_change("placeholder", e.placeholder.get());
        e.placeholder.changed.subscribe(placeholder_slot_, placeholder_cb_);
    }

    void map_is_password(basic_entry& e) {
        record_change("is_password", e.is_password.get());
        e.is_password.changed.subscribe(is_password_slot_, is_password_cb_);
    }

    void map_is_read_only(basic_entry& e) {
        record_change("is_read_only", e.is_read_only.get());
        e.is_read_only.changed.subscribe(is_read_only_slot_, is_read_only_cb_);
    }

    void map_semantics(basic_entry& e) {
        record_change("semantic_description", e.semantic_description.get());
        e.semantic_description.changed.subscribe(sem_slot_, sem_cb_);
    }

    void map_max_length(basic_entry& e) {
        record_change("max_length", e.max_length.get());
        e.max_length.changed.subscribe(max_length_slot_, max_length_cb_);
    }

    void map_cursor_position(basic_entry& e) {
        record_change("cursor_position", e.cursor_position.get());
        e.cursor_position.changed.subscribe(cursor_position_slot_,
                                            cursor_position_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_entry& /*x*/) noexcept {}


private:
    using self_t = entry_handler<platform::mock>;

    mock_property_recorder<self_t, std::string> text_cb_{this, "text"};
    signal_slot<const std::string&>             text_slot_{};

    mock_property_recorder<self_t, std::string> placeholder_cb_{this,
                                                                "placeholder"};
    signal_slot<const std::string&>             placeholder_slot_{};

    mock_property_recorder<self_t, bool>        is_password_cb_{this,
                                                                "is_password"};
    signal_slot<const bool&>                    is_password_slot_{};

    mock_property_recorder<self_t, bool>        is_read_only_cb_{this,
                                                                 "is_read_only"};
    signal_slot<const bool&>                    is_read_only_slot_{};

    mock_property_recorder<self_t, std::string> sem_cb_{this,
                                                        "semantic_description"};
    signal_slot<const std::string&>             sem_slot_{};

    mock_property_recorder<self_t, int>         max_length_cb_{this,
                                                               "max_length"};
    signal_slot<const int&>                     max_length_slot_{};

    mock_property_recorder<self_t, int>         cursor_position_cb_{
        this, "cursor_position"};
    signal_slot<const int&>                     cursor_position_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_ENTRY_HANDLER_HPP
