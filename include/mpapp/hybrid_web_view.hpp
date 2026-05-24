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

#include <coroutine>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "detail/json.hpp"
#include "hybrid_bridge.hpp"
#include "observable.hpp"
#include "platform.hpp"
#include "signal.hpp"
#include "web_view.hpp"

namespace mpapp {

template <class Platform = platform::current>
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

    // HTML content for the embedded page. The real per-platform
    // handlers wire this to webkit_web_view_load_html / WebView2
    // NavigateToString / Android WebView.loadDataWithBaseURL. Setting
    // an empty string clears the page; setting non-empty triggers a
    // navigation to that HTML (typically reported as `about:blank`
    // by the navigation-event side, depending on platform). Apps that
    // need to load a remote URL instead can wrap a plain
    // mpapp::web_view alongside this control.
    Observable<std::string> html_source{""};

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
    // native messaging path. Tri-state envelope classification:
    //
    //   * Envelope with a "method" field    →  inbound bridge call.
    //     Dispatch through bridge_ if attached; post the response
    //     back via send_to_js.
    //
    //   * Envelope with a "result"/"error" field + matching pending
    //     callback id  →  response to an earlier invoke_js_cb.
    //     Invoke the callback with the parsed result; remove from
    //     pending_responses_.
    //
    //   * Anything else  →  emit `message_received` (raw-string path).
    //
    // last_message_in is set in every path so debug observers see the
    // raw traffic regardless.
    void process_inbound(const std::string& payload) {
        last_message_in.set(payload);

        if (!payload.empty() && payload.front() == '{') {
            // Classify envelope by walking it once.
            int  envelope_id          = -1;
            bool has_method           = false;
            bool has_result_or_error  = false;
            {
                detail::json::reader peek{payload};
                if (peek.expect_object_begin()) {
                    std::string field;
                    while (peek.next_field(field)) {
                        if      (field == "method") { has_method = true; (void)peek.skip_value(); }
                        else if (field == "result" || field == "error") {
                            has_result_or_error = true;
                            (void)peek.skip_value();
                        }
                        else if (field == "id")     { (void)peek.read(envelope_id); }
                        else                         { (void)peek.skip_value(); }
                    }
                }
            }

            if (has_method && bridge_ != nullptr) {
                // Route through dispatch_async — sync methods fire the
                // callback inline (preserving v1 behavior); async
                // methods defer it until their respond() callback
                // resolves.
                bridge_->dispatch_async(payload,
                    [this](std::string response) {
                        send_to_js(response);
                    });
                return;
            }
            if (has_result_or_error) {
                auto it = pending_responses_.find(envelope_id);
                if (it != pending_responses_.end()) {
                    auto cb = std::move(it->second);
                    pending_responses_.erase(it);
                    cb(payload);
                    return;
                }
            }
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

    // Same as invoke_js, but registers a callback to receive the JS-
    // side response. The callback fires with `std::optional<T>` —
    // empty if JS posted an `error` envelope or the response couldn't
    // be parsed, populated with the parsed `result` value otherwise.
    //
    // Callbacks fire once and are then discarded. If no response ever
    // arrives, the callback leaks until the hybrid_web_view is
    // destroyed.
    //
    // T must have a json::read overload (primitives, std::string,
    // std::vector<T>, std::optional<T>, ADL-extended user types).
    template <class T, class... Args>
    int invoke_js_cb(std::string_view method_name,
                     std::function<void(std::optional<T>)> on_result,
                     const Args&... args) {
        const int id = next_outbound_id_++;

        // Wrap the user callback into a string-parsing callback that
        // pending_responses_ stores by id.
        auto wrapped = [user_cb = std::move(on_result)](const std::string& response_payload) {
            std::optional<T> parsed;
            bool             had_error = false;
            detail::json::reader r{response_payload};
            if (r.expect_object_begin()) {
                std::string field;
                while (r.next_field(field)) {
                    if (field == "result") {
                        T v{};
                        if (r.read(v)) parsed = std::move(v);
                        // If the read fails we leave `parsed` empty — the
                        // caller treats that the same as an error response.
                    } else if (field == "error") {
                        had_error = true;
                        (void)r.skip_value();
                    } else {
                        (void)r.skip_value();
                    }
                }
            }
            if (had_error) parsed.reset();
            user_cb(std::move(parsed));
        };
        pending_responses_.emplace(id, std::move(wrapped));

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

    // How many invoke_js_cb calls are awaiting a response. For tests
    // and leak debugging.
    [[nodiscard]] std::size_t pending_response_count() const noexcept {
        return pending_responses_.size();
    }

    // Coroutine-friendly version of invoke_js_cb. Returns an awaiter
    // that resumes the calling coroutine with `std::optional<T>` once
    // the JS side posts a matching response envelope.
    //
    // Usage:
    //   mpapp::task<int> add_via_bridge(hybrid_web_view& wv) {
    //       auto r = co_await wv.invoke_js_async<int>("add", 1, 2);
    //       co_return r.value_or(-1);
    //   }
    //
    // The result is `std::optional<T>` to match invoke_js_cb's
    // semantics: empty means JS posted an error envelope or the
    // result couldn't be parsed as T.
    template <class T, class... Args>
    [[nodiscard]] auto invoke_js_async(std::string_view method_name, const Args&... args) {
        // Awaiter — stores enough state to register the callback at
        // await_suspend time, then yield std::optional<T> at resume.
        struct awaiter {
            hybrid_web_view*                     self;
            std::string                          method;
            std::tuple<std::decay_t<Args>...>    args_tuple;
            std::optional<T>                     result_slot;

            bool await_ready() const noexcept { return false; }

            void await_suspend(std::coroutine_handle<> h) {
                std::apply([this, h](auto&... a) {
                    self->template invoke_js_cb<T>(
                        method,
                        [this, h](std::optional<T> r) mutable {
                            result_slot = std::move(r);
                            h.resume();
                        },
                        a...);
                }, args_tuple);
            }

            std::optional<T> await_resume() noexcept {
                return std::move(result_slot);
            }
        };
        return awaiter{this, std::string{method_name},
                       std::tuple<std::decay_t<Args>...>{args...},
                       std::nullopt};
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
    // Pending outbound calls keyed by id. The value is invoked exactly
    // once when the JS side posts a matching {"id":N,"result"|"error":…}
    // envelope.
    std::unordered_map<int, std::function<void(const std::string& /*response_payload*/)>>
                                                pending_responses_{};
    hybrid_web_view_handler<platform::current>* hwv_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_HYBRID_WEB_VIEW_HPP
