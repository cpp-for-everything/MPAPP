// SPDX-License-Identifier: Apache-2.0
// Mock basic_content_view handler.

#ifndef MPAPP_HANDLERS_MOCK_CONTENT_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_CONTENT_VIEW_HANDLER_HPP

#include <memory>

#include "../../internal/basic_content_view.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class content_view_handler<platform::mock>
    : public mock_handler_base {
public:
    content_view_handler() = default;

    void map_content(basic_content_view& c) {
        record("content", c.content.get() ? std::string("set") : std::string("null"));
        c.content.changed.subscribe(content_slot_, content_cb_);
    }

private:
    struct content_cb_t {
        content_view_handler<platform::mock>* self;
        void operator()(const std::shared_ptr<view>& v) const {
            self->record("content", v ? std::string("set") : std::string("null"));
        }
    };

    content_cb_t                              content_cb_{this};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_CONTENT_VIEW_HANDLER_HPP
