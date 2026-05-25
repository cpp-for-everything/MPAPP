// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-platform specialisation of `editor_handler`.

#ifndef MPAPP_HANDLERS_MOCK_EDITOR_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_EDITOR_HANDLER_HPP

#include <string>

#include "../../internal/basic_editor.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class editor_handler<platform::mock> : public mock_handler_base {
public:
    editor_handler()  = default;
    ~editor_handler() = default;

    editor_handler(const editor_handler&)            = delete;
    editor_handler& operator=(const editor_handler&) = delete;
    editor_handler(editor_handler&&)                 = delete;
    editor_handler& operator=(editor_handler&&)      = delete;

    void map_text(basic_editor& e) {
        record_change("text", e.text.get());
        e.text.changed.subscribe(text_slot_, text_cb_);
    }

    void map_placeholder(basic_editor& e) {
        record_change("placeholder", e.placeholder.get());
        e.placeholder.changed.subscribe(placeholder_slot_, placeholder_cb_);
    }

    void map_is_read_only(basic_editor& e) {
        record_change("is_read_only", e.is_read_only.get());
        e.is_read_only.changed.subscribe(is_read_only_slot_, is_read_only_cb_);
    }

    void map_max_length(basic_editor& e) {
        record_change("max_length", e.max_length.get());
        e.max_length.changed.subscribe(max_length_slot_, max_length_cb_);
    }

private:
    using self_t = editor_handler<platform::mock>;

    mock_property_recorder<self_t, std::string> text_cb_{this, "text"};
    signal_slot<const std::string&>             text_slot_{};

    mock_property_recorder<self_t, std::string> placeholder_cb_{this,
                                                                "placeholder"};
    signal_slot<const std::string&>             placeholder_slot_{};

    mock_property_recorder<self_t, bool>        is_read_only_cb_{this,
                                                                 "is_read_only"};
    signal_slot<const bool&>                    is_read_only_slot_{};

    mock_property_recorder<self_t, int>         max_length_cb_{this,
                                                               "max_length"};
    signal_slot<const int&>                     max_length_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_EDITOR_HANDLER_HPP
