// SPDX-License-Identifier: Apache-2.0
// Mock basic_image handler.

#ifndef MPAPP_HANDLERS_MOCK_IMAGE_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_IMAGE_HANDLER_HPP

#include <string>

#include "../../internal/basic_image.hpp"
#include "../../internal/basic_image_source.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"
#include "image_loader.hpp"   // for image_loader<platform::mock>

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

    // RFC-0004 rich-source mapper. Records the initial value via the
    // owned `image_loader<platform::mock>` + subscribes a slot so
    // subsequent `source_object.set(...)` calls record too. Tests can
    // inspect `loader().calls()` to assert exactly which sources were
    // loaded + in what order.
    void map_source_object(basic_image& i) {
        loader_.load(i.source_object.get());
        i.source_object.changed.subscribe(source_object_slot_, source_object_cb_);
    }

    // Inspector for tests — exposes the mock loader so callers can
    // assert `h.loader().calls_as_strings() == ...`.
    [[nodiscard]] ::mpapp::image_loader<::mpapp::platform::mock>&
    loader() noexcept { return loader_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_image& /*x*/) noexcept {}


private:
    struct aspect_cb_t {
        image_handler<platform::mock>* self;
        void operator()(aspect_mode m) const {
            self->record("aspect", static_cast<int>(m));
        }
    };

    struct source_object_cb_t {
        image_handler<platform::mock>* self;
        void operator()(const ::mpapp::image_source_ref& s) const {
            self->loader_.load(s);
        }
    };

    detail::property_binding<std::string>     binding_source_{};
    aspect_cb_t                               aspect_cb_{this};
    signal_slot<const aspect_mode&>           aspect_slot_{};

    // RFC-0004 rich-source state. The loader is owned by the handler
    // so its `calls()` log mirrors the per-handler test pattern (one
    // image_handler instance per test ⇒ isolated load log).
    ::mpapp::image_loader<::mpapp::platform::mock>
                                              loader_{};
    source_object_cb_t                        source_object_cb_{this};
    signal_slot<const ::mpapp::image_source_ref&>
                                              source_object_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_IMAGE_HANDLER_HPP
