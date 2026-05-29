// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-platform specialisation of `label_handler`.

#ifndef MPAPP_HANDLERS_MOCK_LABEL_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_LABEL_HANDLER_HPP

#include <string>

#include "../../internal/basic_label.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class label_handler<platform::mock> : public mock_handler_base {
public:
    label_handler()  = default;
    ~label_handler() = default;

    label_handler(const label_handler&)            = delete;
    label_handler& operator=(const label_handler&) = delete;
    label_handler(label_handler&&)                 = delete;
    label_handler& operator=(label_handler&&)      = delete;

    void map_text(basic_label& l) {
        record_change("text", l.text.get());
        l.text.changed.subscribe(text_slot_, text_cb_);
    }

    void map_font_size(basic_label& l) {
        record_change("font_size", l.font_size.get());
        l.font_size.changed.subscribe(font_size_slot_, font_size_cb_);
    }

    void map_font_bold(basic_label& l) {
        record_change("font_bold", l.font_bold.get());
        l.font_bold.changed.subscribe(font_bold_slot_, font_bold_cb_);
    }

    void map_font_family(basic_label& l) {
        record_change("font_family", l.font_family.get());
        l.font_family.changed.subscribe(font_family_slot_, font_family_cb_);
    }

    void map_text_color(basic_label& l) {
        record_change("text_color", l.text_color.get());
        l.text_color.changed.subscribe(text_color_slot_, text_color_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_label& /*x*/) noexcept {}


private:
    mock_property_recorder<label_handler<platform::mock>, std::string> text_cb_{
        this, "text"};
    signal_slot<const std::string&> text_slot_{};

    mock_property_recorder<label_handler<platform::mock>, double> font_size_cb_{
        this, "font_size"};
    signal_slot<const double&> font_size_slot_{};

    mock_property_recorder<label_handler<platform::mock>, bool> font_bold_cb_{
        this, "font_bold"};
    signal_slot<const bool&> font_bold_slot_{};

    mock_property_recorder<label_handler<platform::mock>, std::string> font_family_cb_{
        this, "font_family"};
    signal_slot<const std::string&> font_family_slot_{};

    mock_property_recorder<label_handler<platform::mock>, color> text_color_cb_{
        this, "text_color"};
    signal_slot<const color&> text_color_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_LABEL_HANDLER_HPP
