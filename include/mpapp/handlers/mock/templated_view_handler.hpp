// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/TemplatedView.md
//
// `templated_view_handler<platform::mock>` — records the two mappers
// (content / template_id). Inherits `mock_handler_base`. Used in unit
// tests and on host platforms without a native UI toolkit.

#ifndef MPAPP_HANDLERS_MOCK_TEMPLATED_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_TEMPLATED_VIEW_HANDLER_HPP

#include <memory>
#include <string>

#include "../../platform.hpp"
#include "../../internal/basic_templated_view.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class templated_view_handler<platform::mock>
    : public mock_handler_base {
public:
    templated_view_handler() = default;

    void map_content(basic_templated_view& t) {
        record("content", t.content.get() ? std::string("set") : std::string("null"));
        t.content.changed.subscribe(content_slot_, content_cb_);
    }

    void map_template_id(basic_templated_view& t) {
        record("template_id", t.template_id.get());
        t.template_id.changed.subscribe(template_id_slot_, template_id_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_templated_view& /*x*/) noexcept {}


private:
    using self_t = templated_view_handler<platform::mock>;

    struct content_cb_t {
        self_t* self;
        void operator()(const std::shared_ptr<view>& v) const {
            self->record("content", v ? std::string("set") : std::string("null"));
        }
    };
    struct template_id_cb_t {
        self_t* self;
        void operator()(const std::string& v) const { self->record("template_id", v); }
    };

    content_cb_t                              content_cb_{this};
    template_id_cb_t                          template_id_cb_{this};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
    signal_slot<const std::string&>           template_id_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_TEMPLATED_VIEW_HANDLER_HPP
