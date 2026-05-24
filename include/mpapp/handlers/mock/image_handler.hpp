// SPDX-License-Identifier: Apache-2.0
// Mock basic_image handler.

#ifndef MPAPP_HANDLERS_MOCK_IMAGE_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_IMAGE_HANDLER_HPP

#include <string>

#include "../../internal/basic_image.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class image_handler<platform::mock>
    : public mock_handler_base {
public:
    image_handler() = default;

    void map_source(basic_image& i) { bind("source", i.source, binding_source_); }
    void map_aspect(basic_image& i) {
        record("aspect", static_cast<int>(i.aspect.get()));
        i.aspect.changed.subscribe(aspect_slot_, aspect_cb_);
    }

private:
    struct aspect_cb_t {
        image_handler<platform::mock>* self;
        void operator()(aspect_mode m) const {
            self->record("aspect", static_cast<int>(m));
        }
    };

    detail::property_binding<std::string>     binding_source_{};
    aspect_cb_t                               aspect_cb_{this};
    signal_slot<const aspect_mode&>           aspect_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_IMAGE_HANDLER_HPP
