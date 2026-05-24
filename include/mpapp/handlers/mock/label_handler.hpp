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

private:
    mock_property_recorder<label_handler<platform::mock>, std::string> text_cb_{
        this, "text"};
    signal_slot<const std::string&> text_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_LABEL_HANDLER_HPP
