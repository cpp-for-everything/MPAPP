// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. `hybrid_web_view` extends web_view with a C++ <-> JS
// message bridge.
//
// Two layers ride on the same inbound/outbound pipe:
//
//   1. **Raw string bridge** (always available):
//      `send_to_js(payload)` posts a string to JS;
//      `message_received` fires when JS posts a string back.
//
//   2. **Typed JSON-RPC bridge** (opt-in, ADR-0018):
//      `set_bridge<MyBridge>()` registers a `mpapp::hybrid_bridge`
//      subclass with method registrations. Inbound JSON envelopes
//      get dispatched through the bridge; the response is posted
//      back via `send_to_js` automatically. Non-envelope traffic
//      and traffic that doesn't start with `{` still surfaces on
//      `message_received` as before.
//
// Platform handlers feed inbound traffic through a single
// `process_inbound()` choke point so the bridge decision lives in
// the cross-platform class, not duplicated across Win/Linux/Android.

#ifndef MPAPP_HYBRID_WEB_VIEW_HPP
#define MPAPP_HYBRID_WEB_VIEW_HPP

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "hybrid_bridge.hpp"
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

    // Inbound message from JS. Fires for traffic NOT consumed by an
    // attached `hybrid_bridge`. If a bridge is attached and the
    // payload begins with `{` (JSON envelope), the bridge handles it
    // and message_received does NOT fire — bridge consumers should
    // observe responses on `message_sent` and inbound on the bridge
    // method calls themselves.
    signal<const std::string&> message_received{};

    // Outbound message — fired whenever `send_to_js` posts. Includes
    // bridge responses.
    signal<const std::string&> message_sent{};

    // ----- Mutators -----------------------------------------------------

    // Send a string payload to JS. The real handler posts it through
    // the platform's native messaging path.
    void send_to_js(const std::string& payload) {
        last_message_out_ = payload;
        message_sent.emit(payload);
    }

    // Convenience used by tests + the mock handler to simulate the JS
    // side calling back into us. Real handlers call `process_inbound`
    // directly with the platform-native payload.
    void simulate_inbound(const std::string& payload) {
        process_inbound(payload);
    }

    // The single inbound choke point. Platform handlers (Win, Linux,
    // Android) call this with the raw payload they received from the
    // native messaging path. We:
    //   * record `last_message_in` for debug observers,
    //   * if a bridge is attached AND the payload looks like a JSON
    //     envelope, dispatch through it and post the response back
    //     via `send_to_js`,
    //   * otherwise fire `message_received` (raw-string path).
    void process_inbound(const std::string& payload) {
        last_message_in.set(payload);
        if (bridge_ != nullptr && !payload.empty() && payload.front() == '{') {
            std::string response;
            (void)bridge_->dispatch(payload, response);
            // dispatch() always writes a well-formed envelope — success
            // or error — so the JS side can match by id either way.
            send_to_js(response);
            return;
        }
        message_received.emit(payload);
    }

    [[nodiscard]] const std::string& last_message_out() const noexcept { return last_message_out_; }

    // ----- Typed bridge (ADR-0018) --------------------------------------

    // Attach a bridge. `T` must derive from `mpapp::hybrid_bridge`.
    // Constructs the bridge in-place and stores it; the bridge lives
    // for the hybrid_web_view's lifetime. Returns a reference so the
    // caller can configure additional state on the bridge after
    // attachment.
    template <class T, class... Args>
    T& set_bridge(Args&&... args) {
        static_assert(std::is_base_of_v<hybrid_bridge, T>,
                      "set_bridge<T>(): T must derive from mpapp::hybrid_bridge");
        auto p = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *p;
        bridge_ = std::move(p);
        return ref;
    }

    [[nodiscard]] hybrid_bridge*       bridge()       noexcept { return bridge_.get(); }
    [[nodiscard]] const hybrid_bridge* bridge() const noexcept { return bridge_.get(); }
    [[nodiscard]] bool                 has_bridge()   const noexcept { return bridge_ != nullptr; }

    // Invoke a JS-side method with typed arguments. Constructs a JSON-RPC
    // envelope of the form
    //   {"id":N,"method":"<method_name>","args":[<arg0>,<arg1>,...]}
    // and posts it through send_to_js. The id auto-increments per call
    // so the JS side (or its eventual codegen'd response router) can
    // correlate.
    //
    // v1 is fire-and-forget — there's no return-value plumbing yet.
    // The `task<T>` async-return shape from ADR-0018 § Decision is v2.
    //
    // The args' types must each have a `write` overload in the json
    // layer (primitives, std::string, std::vector<T>, std::optional<T>,
    // or user types that overload `to_json(writer&, const T&)` via ADL).
    //
    // Returns the id used in the envelope so the caller can match
    // responses on `message_received` (typed-response routing on the
    // C++ side is also v2).
    template <class... Args>
    int invoke_js(std::string_view method_name, const Args&... args) {
        const int id = next_outbound_id_++;
        std::string envelope;
        {
            detail::json::writer w{envelope};
            w.begin_object();
            w.field("id",     id);
            w.field("method", method_name);
            w.field_array("args", args...);
            w.end_object();
        }
        send_to_js(envelope);
        return id;
    }

    // ----- Handler ------------------------------------------------------

    hybrid_web_view_handler<platform::current>&       hwv_handler() noexcept       { return *hwv_handler_; }
    const hybrid_web_view_handler<platform::current>& hwv_handler() const noexcept { return *hwv_handler_; }
    bool                                              has_hwv_handler() const noexcept { return hwv_handler_ != nullptr; }
    void                                              set_hwv_handler(hybrid_web_view_handler<platform::current>& h) noexcept { hwv_handler_ = &h; }

private:
    std::string                                 last_message_out_{};
    std::unique_ptr<hybrid_bridge>              bridge_{};
    int                                         next_outbound_id_ = 1;
    hybrid_web_view_handler<platform::current>* hwv_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_HYBRID_WEB_VIEW_HPP
