// SPDX-License-Identifier: Apache-2.0
// Mock handler for `mpapp::basic_image_cell`. Extends basic_text_cell with image_uri.

#ifndef MPAPP_HANDLERS_MOCK_IMAGE_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_IMAGE_CELL_HANDLER_HPP

#include <string>

#include "../../internal/basic_image_cell.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class image_cell_handler<platform::mock> : public mock_handler_base {
public:
    image_cell_handler() = default;
    ~image_cell_handler() = default;

    image_cell_handler(const image_cell_handler&)            = delete;
    image_cell_handler& operator=(const image_cell_handler&) = delete;
    image_cell_handler(image_cell_handler&&)                 = delete;
    image_cell_handler& operator=(image_cell_handler&&)      = delete;

    void map_text(basic_image_cell& c) {
        record_change("text", c.text.get());
        c.text.changed.subscribe(text_slot_, text_cb_);
    }
    void map_image_uri(basic_image_cell& c) {
        record_change("image_uri", c.image_uri.get());
        c.image_uri.changed.subscribe(uri_slot_, uri_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_image_cell& /*x*/) noexcept {}


private:
    using self_t = image_cell_handler<platform::mock>;
    mock_property_recorder<self_t, std::string> text_cb_{this, "text"};
    mock_property_recorder<self_t, std::string> uri_cb_{this, "image_uri"};
    signal_slot<const std::string&>             text_slot_{};
    signal_slot<const std::string&>             uri_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_IMAGE_CELL_HANDLER_HPP
