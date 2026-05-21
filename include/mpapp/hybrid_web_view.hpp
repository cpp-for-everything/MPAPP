// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. `hybrid_web_view` extends web_view with a C++ <-> JS
// message bridge. Mock surface keeps two signals (one each direction)
// and an invoke_js() helper that just records the call for tests.

#ifndef MPAPP_HYBRID_WEB_VIEW_HPP
#define MPAPP_HYBRID_WEB_VIEW_HPP

#include <string>
#include <vector>

#include "observable.hpp"
#include "platform.hpp"
#include "signal.hpp"
#include "web_view.hpp"

namespace mpapp {

template <class Platform>
class hybrid_web_view_handler;

class hybrid_web_view : public web_view {
public:
    hybrid_web_view() = default;
    ~hybrid_web_view() override = default;

    hybrid_web_view(const hybrid_web_view&)            = delete;
    hybrid_web_view& operator=(const hybrid_web_view&) = delete;
    hybrid_web_view(hybrid_web_view&&)                 = delete;
    hybrid_web_view& operator=(hybrid_web_view&&)      = delete;

    // ----- JS bridge surface --------------------------------------------

    // Bound JS namespace; the real handler exposes this as window[hybrid_namespace] on the JS side.
    Observable<std::string> hybrid_namespace{"mpapp"};

    // Most-recent inbound message (JS -> C++).
    Observable<std::string> last_message_in{""};

    // ----- Signals ------------------------------------------------------

    // Inbound message from JS. The handler emits this whenever the JS
    // side calls `window.mpapp.send(payload)`.
    signal<const std::string&> message_received{};

    // Outbound message ack. Emitted by send_to_js() so tests can
    // assert "we sent X downstream."
    signal<const std::string&> message_sent{};

    // ----- Mutators -----------------------------------------------------

    // Send a string payload to the JS bridge. Mock just records it.
    void send_to_js(const std::string& payload) {
        last_message_out_ = payload;
        message_sent.emit(payload);
    }

    // Convenience: simulate the JS side calling back into us.
    void simulate_inbound(const std::string& payload) {
        last_message_in.set(payload);
        message_received.emit(payload);
    }

    [[nodiscard]] const std::string& last_message_out() const noexcept { return last_message_out_; }

    // ----- Handler ------------------------------------------------------

    hybrid_web_view_handler<platform::current>&       hwv_handler() noexcept       { return *hwv_handler_; }
    const hybrid_web_view_handler<platform::current>& hwv_handler() const noexcept { return *hwv_handler_; }
    bool                                              has_hwv_handler() const noexcept { return hwv_handler_ != nullptr; }
    void                                              set_hwv_handler(hybrid_web_view_handler<platform::current>& h) noexcept { hwv_handler_ = &h; }

private:
    std::string                                last_message_out_{};
    hybrid_web_view_handler<platform::current>* hwv_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_HYBRID_WEB_VIEW_HPP
