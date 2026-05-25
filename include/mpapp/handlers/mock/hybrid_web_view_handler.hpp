// SPDX-License-Identifier: Apache-2.0
// Mock handler for `mpapp::basic_hybrid_web_view`.

#ifndef MPAPP_HANDLERS_MOCK_HYBRID_WEB_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_HYBRID_WEB_VIEW_HANDLER_HPP

#include <string>

#include "../../internal/basic_hybrid_web_view.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class hybrid_web_view_handler<platform::mock> : public mock_handler_base {
public:
    hybrid_web_view_handler() = default;
    ~hybrid_web_view_handler() = default;

    hybrid_web_view_handler(const hybrid_web_view_handler&)            = delete;
    hybrid_web_view_handler& operator=(const hybrid_web_view_handler&) = delete;
    hybrid_web_view_handler(hybrid_web_view_handler&&)                 = delete;
    hybrid_web_view_handler& operator=(hybrid_web_view_handler&&)      = delete;

    void map_messages(basic_hybrid_web_view& h) {
        h.message_received.subscribe(slot_recv_, recv_cb_);
        h.message_sent.subscribe(slot_sent_, sent_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_hybrid_web_view& /*x*/) noexcept {}


private:
    using self_t = hybrid_web_view_handler<platform::mock>;

    struct recv_recorder {
        self_t* self = nullptr;
        void operator()(const std::string& v) const { self->record_change("message_received", v); }
    };
    struct sent_recorder {
        self_t* self = nullptr;
        void operator()(const std::string& v) const { self->record_change("message_sent", v); }
    };

    recv_recorder recv_cb_{this};
    sent_recorder sent_cb_{this};

    signal_slot<const std::string&> slot_recv_{};
    signal_slot<const std::string&> slot_sent_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_HYBRID_WEB_VIEW_HANDLER_HPP
