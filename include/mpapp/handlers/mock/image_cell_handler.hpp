// SPDX-License-Identifier: Apache-2.0
// Mock handler for `mpapp::image_cell`. Extends text_cell with image_uri.

#ifndef MPAPP_HANDLERS_MOCK_IMAGE_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_IMAGE_CELL_HANDLER_HPP

#include <string>

#include "../../image_cell.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class image_cell_handler<platform::mock> : public mock_handler_base {
public:
    image_cell_handler() = default;
    ~image_cell_handler() = default;

    image_cell_handler(const image_cell_handler&)            = delete;
    image_cell_handler& operator=(const image_cell_handler&) = delete;
    image_cell_handler(image_cell_handler&&)                 = delete;
    image_cell_handler& operator=(image_cell_handler&&)      = delete;

    void map_text(image_cell& c) {
        record_change("text", c.text.get());
        c.text.changed.subscribe(text_slot_, text_cb_);
    }
    void map_image_uri(image_cell& c) {
        record_change("image_uri", c.image_uri.get());
        c.image_uri.changed.subscribe(uri_slot_, uri_cb_);
    }

private:
    using self_t = image_cell_handler<platform::mock>;
    mock_property_recorder<self_t, std::string> text_cb_{this, "text"};
    mock_property_recorder<self_t, std::string> uri_cb_{this, "image_uri"};
    signal_slot<const std::string&>             text_slot_{};
    signal_slot<const std::string&>             uri_slot_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_IMAGE_CELL_HANDLER_HPP
