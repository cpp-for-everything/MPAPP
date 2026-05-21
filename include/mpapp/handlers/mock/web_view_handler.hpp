// SPDX-License-Identifier: Apache-2.0
// Mock handler for `mpapp::web_view`.

#ifndef MPAPP_HANDLERS_MOCK_WEB_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_WEB_VIEW_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../web_view.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class web_view_handler<platform::mock> : public mock_handler_base {
public:
    web_view_handler() = default;
    ~web_view_handler() = default;

    web_view_handler(const web_view_handler&)            = delete;
    web_view_handler& operator=(const web_view_handler&) = delete;
    web_view_handler(web_view_handler&&)                 = delete;
    web_view_handler& operator=(web_view_handler&&)      = delete;

    void map_url(web_view& wv) {
        record_change("url", wv.url.get());
        wv.url.changed.subscribe(slot_url_, url_cb_);
    }

    void map_is_loading(web_view& wv) {
        record_change("is_loading", wv.is_loading.get());
        wv.is_loading.changed.subscribe(slot_load_, load_cb_);
    }

private:
    using self_t = web_view_handler<platform::mock>;

    struct url_recorder {
        self_t* self = nullptr;
        void operator()(const std::string& v) const { self->record_change("url", v); }
    };
    struct load_recorder {
        self_t* self = nullptr;
        void operator()(bool v) const { self->record_change("is_loading", v); }
    };

    url_recorder  url_cb_{this};
    load_recorder load_cb_{this};

    signal_slot<const std::string&> slot_url_{};
    signal_slot<const bool&>        slot_load_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_WEB_VIEW_HANDLER_HPP
