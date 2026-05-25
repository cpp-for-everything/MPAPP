// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Page.md
//
// `page_handler<platform::mock>` — records the basic_page-level property
// mappers (title / content / is_busy). The view-level mappers come
// from inherited `view_handler<platform::mock>` plumbing when the
// host wires both — basic_page tests in this batch exercise the basic_page surface
// only.

#ifndef MPAPP_HANDLERS_MOCK_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_PAGE_HANDLER_HPP

#include <string>

#include "../../internal/basic_page.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class page_handler<platform::mock> : public mock_handler_base {
public:
    page_handler() = default;
    ~page_handler() = default;

    page_handler(const page_handler&)            = delete;
    page_handler& operator=(const page_handler&) = delete;
    page_handler(page_handler&&)                 = delete;
    page_handler& operator=(page_handler&&)      = delete;

    void map_title(basic_page& p) {
        record_change("title", p.title.get());
        p.title.changed.subscribe(title_slot_, title_cb_);
    }

    void map_content(basic_page& p) {
        record_change("content.present", p.content.get() != nullptr);
        p.content.changed.subscribe(content_slot_, content_cb_);
    }

    void map_is_busy(basic_page& p) {
        record_change("is_busy", p.is_busy.get());
        p.is_busy.changed.subscribe(busy_slot_, busy_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_page& /*x*/) noexcept {}


private:
    using self_t = page_handler<platform::mock>;

    struct content_recorder {
        self_t* self = nullptr;
        void operator()(view* v) const { self->record_change("content.present", v != nullptr); }
    };

    mock_property_recorder<self_t, std::string> title_cb_{this, "title"};
    signal_slot<const std::string&>             title_slot_{};

    content_recorder                            content_cb_{this};
    signal_slot<view* const&>                   content_slot_{};

    mock_property_recorder<self_t, bool>        busy_cb_{this, "is_busy"};
    signal_slot<const bool&>                    busy_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_PAGE_HANDLER_HPP
