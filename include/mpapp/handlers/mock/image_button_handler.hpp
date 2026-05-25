// SPDX-License-Identifier: Apache-2.0
// Mock basic_image_button handler.

#ifndef MPAPP_HANDLERS_MOCK_IMAGE_BUTTON_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_IMAGE_BUTTON_HANDLER_HPP

#include <string>

#include "../../internal/basic_image_button.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class image_button_handler<platform::mock>
    : public mock_handler_base {
public:
    image_button_handler() = default;

    void map_source(basic_image_button& b) { bind("source", b.source, binding_source_); }
    void map_aspect(basic_image_button& b) {
        record("aspect", static_cast<int>(b.aspect.get()));
        b.aspect.changed.subscribe(aspect_slot_, aspect_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_image_button& /*x*/) noexcept {}


private:
    struct aspect_cb_t {
        image_button_handler<platform::mock>* self;
        void operator()(aspect_mode m) const {
            self->record("aspect", static_cast<int>(m));
        }
    };

    detail::property_binding<std::string>     binding_source_{};
    aspect_cb_t                               aspect_cb_{this};
    signal_slot<const aspect_mode&>           aspect_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_IMAGE_BUTTON_HANDLER_HPP
