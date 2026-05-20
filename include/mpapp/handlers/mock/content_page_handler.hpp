// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ContentPage.md
//
// `content_page_handler<platform::mock>` — records the three page-level
// mappers (title / content / padding). Inherits `mock_handler_base`.

#ifndef MPAPP_HANDLERS_MOCK_CONTENT_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_CONTENT_PAGE_HANDLER_HPP

#include <memory>
#include <string>

#include "../../content_page.hpp"
#include "../../layout.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class content_page_handler<platform::mock>
    : public mock_handler_base {
public:
    content_page_handler() = default;

    void map_title(content_page& p) {
        record("title", p.title.get());
        p.title.changed.subscribe(title_slot_, title_cb_);
    }

    void map_content(content_page& p) {
        record("content", p.content.get() ? std::string("set") : std::string("null"));
        p.content.changed.subscribe(content_slot_, content_cb_);
    }

    void map_padding(content_page& p) {
        record("padding", p.padding.get());
        p.padding.changed.subscribe(padding_slot_, padding_cb_);
    }

private:
    using self_t = content_page_handler<platform::mock>;

    struct title_cb_t {
        self_t* self;
        void operator()(const std::string& v) const { self->record("title", v); }
    };
    struct content_cb_t {
        self_t* self;
        void operator()(const std::shared_ptr<view>& v) const {
            self->record("content", v ? std::string("set") : std::string("null"));
        }
    };
    struct padding_cb_t {
        self_t* self;
        void operator()(const thickness& t) const { self->record("padding", t); }
    };

    title_cb_t                                title_cb_{this};
    content_cb_t                              content_cb_{this};
    padding_cb_t                              padding_cb_{this};
    signal_slot<const std::string&>           title_slot_{};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
    signal_slot<const thickness&>             padding_slot_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_CONTENT_PAGE_HANDLER_HPP
