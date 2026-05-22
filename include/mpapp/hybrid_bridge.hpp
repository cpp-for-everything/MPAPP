// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0018-hybrid-webview-typed-bridge.md
//
// `mpapp::hybrid_bridge` — base class for HybridWebView typed bridges.
// Apps subclass it, register methods in the constructor, and pass the
// bridge to a `hybrid_web_view` via `wv->set_bridge<my_bridge>()`.
//
//   class my_bridge : public mpapp::hybrid_bridge {
//   public:
//       my_bridge() {
//           register_method("add",       &my_bridge::add);
//           register_method("greet",     &my_bridge::greet);
//       }
//
//       int         add(int a, int b)             { return a + b; }
//       std::string greet(const std::string& name) { return "hi " + name; }
//   };
//
// Wire format on inbound JSON-RPC envelope:
//   { "id": <int>, "method": <string>, "args": [<arg0>, <arg1>, ...] }
//
// Wire format on response:
//   { "id": <int>, "result": <value> }       — success
//   { "id": <int>, "error": "<message>" }    — error
//
// v1 is synchronous: bridge methods return their result by value,
// `dispatch()` writes the response envelope inline, and the platform
// handler posts that response back to JS via the existing
// `send_to_js` path. Async return values (the `task<T>` shape from
// ADR-0018 §Decision) are v2 — the surface here doesn't preclude them
// but doesn't ship them either.
//
// Codegen tool (`mpapp-bridge-gen`) is M-09 scope. For now methods
// register themselves manually in the constructor; the
// `[[mpapp::js_method]]` attribute is just a marker the future tool
// will scan for.

#ifndef MPAPP_HYBRID_BRIDGE_HPP
#define MPAPP_HYBRID_BRIDGE_HPP

#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "detail/json.hpp"

namespace mpapp {

class hybrid_bridge {
public:
    hybrid_bridge() = default;
    virtual ~hybrid_bridge() = default;

    hybrid_bridge(const hybrid_bridge&)            = delete;
    hybrid_bridge& operator=(const hybrid_bridge&) = delete;
    hybrid_bridge(hybrid_bridge&&)                 = delete;
    hybrid_bridge& operator=(hybrid_bridge&&)      = delete;

    // Synchronous dispatch. `payload` is the raw envelope text; `out`
    // receives the response envelope (success or error). Returns true
    // if the envelope was structurally valid and a sync method was
    // found and invoked; false if the envelope was malformed, the
    // method is unknown, or the method is async (use dispatch_async
    // for async methods).
    //
    // `out` is populated with a well-formed JSON envelope even on
    // error so the JS side can match by id.
    bool dispatch(std::string_view payload, std::string& out);

    // Async-capable dispatch. Handles both sync and async methods
    // uniformly. The completion callback fires:
    //   * inline (synchronously) for sync methods — immediately after
    //     the method returns;
    //   * later for async methods — when the user method invokes its
    //     respond() callback.
    //
    // In either case, the callback receives the full response envelope
    // string (success or error), suitable for posting back to JS via
    // send_to_js.
    //
    // `on_response` may capture references to the surrounding
    // hybrid_web_view; for v1, the caller is responsible for ensuring
    // any captured state outlives pending async operations. Typical
    // apps where the hybrid_web_view lives for the app's lifetime
    // satisfy this naturally.
    void dispatch_async(std::string_view payload,
                        std::function<void(std::string /*response*/)> on_response);

    // For tests and introspection. Returns the list of registered
    // method names in registration order.
    [[nodiscard]] std::vector<std::string> method_names() const {
        std::vector<std::string> names;
        names.reserve(methods_.size());
        for (const auto& m : methods_) names.push_back(m.name);
        return names;
    }

protected:
    // Register a method on a derived bridge. `Self` is the derived
    // class (deduced); `Method` is a pointer-to-member-function
    // taking JSON-deserializable args and returning a JSON-
    // serializable value (or void).
    template <class Self, class Ret, class... Args>
    void register_method(std::string name, Ret (Self::*method)(Args...)) {
        methods_.push_back(method_entry{
            std::move(name),
            make_invoker<Self, Ret, Args...>(method),
            {},
        });
    }

    // Const-member overload.
    template <class Self, class Ret, class... Args>
    void register_method(std::string name, Ret (Self::*method)(Args...) const) {
        methods_.push_back(method_entry{
            std::move(name),
            make_invoker_const<Self, Ret, Args...>(method),
            {},
        });
    }

    // Register an async method. The user's method takes Args... + an
    // extra std::function<void(T)> `respond` callback (T = the result
    // type — must be JSON-serializable). The method calls respond()
    // whenever ready — synchronously or from a future callback / timer
    // / async I/O completion. Each method has exactly one valid
    // respond() invocation per call; subsequent calls are silently
    // ignored.
    //
    //   class my_bridge : public hybrid_bridge {
    //   public:
    //       my_bridge() {
    //           register_async_method<int>(
    //               "slow_add",
    //               &my_bridge::slow_add);
    //       }
    //       // Method signature: ...args, std::function<void(T)> respond
    //       void slow_add(int a, int b, std::function<void(int)> respond) {
    //           start_timer(100ms, [a, b, respond]() {
    //               respond(a + b);
    //           });
    //       }
    //   };
    //
    // Implementation note: C++ template deduction can't deduce `Args...`
    // when it appears before a non-pack parameter (`std::function<void(T)>`),
    // even after `T` is explicitly bound. To work around this, the entry
    // point delegates to `async_invoker_builder<T, Method>` — a partial
    // specialization on the *full* member-function-pointer type where the
    // pack appears in trailing (deducible) position; the splitting of the
    // trailing callback off `A...` is then done internally via
    // index_sequence.
    template <class T, class Method>
    void register_async_method(std::string name, Method method) {
        methods_.push_back(method_entry{
            std::move(name),
            {},
            async_invoker_builder<T, Method>::build(method),
        });
    }

private:
    struct method_entry {
        std::string                                              name;
        // Sync invoker: reads args, invokes method, writes result value
        // (just the value, not the full envelope) into result_out.
        // Empty for async methods.
        std::function<bool(hybrid_bridge*, detail::json::reader&,
                           std::string& /*result_value_out*/)> invoke;
        // Async invoker: reads args, invokes method (which captures
        // the envelope-id + on_response continuation and calls it when
        // ready). Empty for sync methods.
        std::function<bool(hybrid_bridge*, detail::json::reader&,
                           int /*id*/,
                           std::function<void(std::string)>)> async_invoke;
    };

    // ---- Invoker construction ------------------------------------------

    template <class Tuple, std::size_t... Is>
    static bool read_tuple(detail::json::reader& r,
                           Tuple& tup,
                           std::index_sequence<Is...>) {
        if (!r.expect_array_begin()) return false;
        bool ok = true;
        // For each slot, advance one element and read into it. The
        // fold-expression preserves short-circuit semantics — a failed
        // read flips `ok` and skips the remaining reads.
        ((ok = ok && r.next_element() && r.read(std::get<Is>(tup))), ...);
        if (!ok) return false;
        // Consume the trailing ']'. next_element returns false at ']'
        // and consumes it.
        return !r.next_element() && r.ok();
    }

    template <class Self, class Ret, class... Args>
    static auto make_invoker(Ret (Self::*method)(Args...)) {
        return [method](hybrid_bridge* base,
                        detail::json::reader& args_r,
                        std::string& result_out) -> bool {
            using arg_tuple = std::tuple<std::decay_t<Args>...>;
            arg_tuple args{};
            if (!read_tuple(args_r, args, std::index_sequence_for<Args...>{})) return false;
            auto* self = static_cast<Self*>(base);
            detail::json::writer w{result_out};
            if constexpr (std::is_void_v<Ret>) {
                std::apply([self, method](auto&&... a) {
                    (self->*method)(std::forward<decltype(a)>(a)...);
                }, args);
                w.write_null();
            } else {
                auto ret = std::apply([self, method](auto&&... a) {
                    return (self->*method)(std::forward<decltype(a)>(a)...);
                }, args);
                w.write(ret);
            }
            return true;
        };
    }

    // ---- Async invoker builder ----------------------------------------
    //
    // The whole point of this partial-specialization dance is to avoid
    // the C++ rule that says "a parameter pack in non-trailing position
    // is non-deducible". We specialize the builder on the *entire*
    // member-function-pointer type `void (Self::*)(A...)` (pack at the
    // end → deducible), then internally split A... into Args... + the
    // trailing std::function<void(T)> via index_sequence over N-1.
    template <class T, class Method>
    struct async_invoker_builder;

    template <class T, class Self, class... A>
    struct async_invoker_builder<T, void (Self::*)(A...)> {
        static_assert(sizeof...(A) >= 1,
                      "register_async_method: method must take at least the "
                      "trailing std::function<void(T)> respond callback");

        using a_tuple = std::tuple<A...>;
        using last_t  = std::tuple_element_t<sizeof...(A) - 1, a_tuple>;
        static_assert(std::is_same_v<last_t, std::function<void(T)>>,
                      "register_async_method: method's trailing argument must "
                      "be exactly std::function<void(T)>");

        template <std::size_t... I>
        static auto build_impl(void (Self::*method)(A...),
                               std::index_sequence<I...>) {
            return [method](hybrid_bridge* base,
                            detail::json::reader& args_r,
                            int id,
                            std::function<void(std::string)> on_response) -> bool {
                using arg_tuple = std::tuple<
                    std::decay_t<std::tuple_element_t<I, a_tuple>>...>;
                arg_tuple args{};
                if (!read_tuple(args_r, args,
                                std::make_index_sequence<sizeof...(I)>{})) return false;
                auto* self = static_cast<Self*>(base);

                // respond callback the user method invokes when ready.
                // Captured shared_ptr<bool> guards against duplicate
                // calls (the second drops on the floor).
                auto fired   = std::make_shared<bool>(false);
                auto respond = [id, on_response = std::move(on_response),
                                fired](T value) {
                    if (*fired) return;
                    *fired = true;
                    std::string envelope;
                    envelope.reserve(32);
                    envelope += "{\"id\":";
                    {
                        char buf[32];
                        const int n = std::snprintf(buf, sizeof(buf), "%d", id);
                        if (n > 0) envelope.append(buf, static_cast<std::size_t>(n));
                    }
                    envelope += ",\"result\":";
                    std::string val;
                    detail::json::writer w{val};
                    w.write(value);
                    envelope += val;
                    envelope += '}';
                    on_response(envelope);
                };

                std::apply([self, method, &respond](auto&&... a) {
                    (self->*method)(std::forward<decltype(a)>(a)..., respond);
                }, args);
                return true;
            };
        }

        static auto build(void (Self::*method)(A...)) {
            return build_impl(method,
                              std::make_index_sequence<sizeof...(A) - 1>{});
        }
    };

    template <class Self, class Ret, class... Args>
    static auto make_invoker_const(Ret (Self::*method)(Args...) const) {
        return [method](hybrid_bridge* base,
                        detail::json::reader& args_r,
                        std::string& result_out) -> bool {
            using arg_tuple = std::tuple<std::decay_t<Args>...>;
            arg_tuple args{};
            if (!read_tuple(args_r, args, std::index_sequence_for<Args...>{})) return false;
            auto* self = static_cast<const Self*>(base);
            detail::json::writer w{result_out};
            if constexpr (std::is_void_v<Ret>) {
                std::apply([self, method](auto&&... a) {
                    (self->*method)(std::forward<decltype(a)>(a)...);
                }, args);
                w.write_null();
            } else {
                auto ret = std::apply([self, method](auto&&... a) {
                    return (self->*method)(std::forward<decltype(a)>(a)...);
                }, args);
                w.write(ret);
            }
            return true;
        };
    }

    static void write_error_envelope(std::string& out, int id, std::string_view message) {
        out.clear();
        detail::json::writer w{out};
        w.begin_object();
        w.field("id",    id);
        w.field("error", message);
        w.end_object();
    }

    // Build a success envelope by splicing `result_json` (which is the
    // already-serialized result value, not the full envelope) inside
    // a {"id":N,"result":...} wrapper. Used by both sync and async
    // success paths.
    static void compose_result_envelope(std::string& out,
                                        int id,
                                        std::string_view result_json) {
        out.clear();
        out.reserve(32 + result_json.size());
        out += "{\"id\":";
        {
            char buf[32];
            const int n = std::snprintf(buf, sizeof(buf), "%d", id);
            if (n > 0) out.append(buf, static_cast<std::size_t>(n));
        }
        out += ",\"result\":";
        out.append(result_json.data(), result_json.size());
        out += '}';
    }

    std::vector<method_entry> methods_;
};

inline bool hybrid_bridge::dispatch(std::string_view payload, std::string& out) {
    out.clear();
    detail::json::reader r{payload};
    if (!r.expect_object_begin()) {
        write_error_envelope(out, -1, "malformed envelope");
        return false;
    }

    int              id = -1;
    std::string      method_name;
    std::string_view args_view;

    std::string field;
    while (r.next_field(field)) {
        if      (field == "id")     { if (!r.read(id))         { write_error_envelope(out, -1, "bad 'id'");     return false; } }
        else if (field == "method") { if (!r.read(method_name)){ write_error_envelope(out, id, "bad 'method'"); return false; } }
        else if (field == "args")   { args_view = r.capture_value(); if (args_view.empty() || !r.ok()) {
                                          write_error_envelope(out, id, "bad 'args'");
                                          return false;
                                      } }
        else                         { if (!r.skip_value())     { write_error_envelope(out, id, "skip failed");  return false; } }
    }
    if (!r.ok()) { write_error_envelope(out, id, "parse error"); return false; }

    if (method_name.empty()) {
        write_error_envelope(out, id, "missing 'method'");
        return false;
    }

    for (auto& m : methods_) {
        if (m.name != method_name) continue;
        if (!m.invoke) {
            // Async-only method invoked via sync dispatch — caller
            // should use dispatch_async for this method.
            write_error_envelope(out, id, "method is async: " + method_name);
            return false;
        }
        detail::json::reader args_r{args_view};
        std::string result;
        if (!m.invoke(this, args_r, result)) {
            write_error_envelope(out, id, "args mismatch for '" + method_name + "'");
            return false;
        }
        // `result` is already JSON text (whatever the method's
        // return-value `write` produced); splice it in unquoted as
        // the value of "result".
        compose_result_envelope(out, id, result);
        return true;
    }

    write_error_envelope(out, id, "unknown method: " + method_name);
    return false;
}

inline void hybrid_bridge::dispatch_async(
    std::string_view payload,
    std::function<void(std::string)> on_response) {
    detail::json::reader r{payload};
    if (!r.expect_object_begin()) {
        std::string err;
        write_error_envelope(err, -1, "malformed envelope");
        on_response(std::move(err));
        return;
    }

    int              id = -1;
    std::string      method_name;
    std::string_view args_view;

    std::string field;
    while (r.next_field(field)) {
        if (field == "id") {
            if (!r.read(id)) {
                std::string err;
                write_error_envelope(err, -1, "bad 'id'");
                on_response(std::move(err));
                return;
            }
        } else if (field == "method") {
            if (!r.read(method_name)) {
                std::string err;
                write_error_envelope(err, id, "bad 'method'");
                on_response(std::move(err));
                return;
            }
        } else if (field == "args") {
            args_view = r.capture_value();
            if (args_view.empty() || !r.ok()) {
                std::string err;
                write_error_envelope(err, id, "bad 'args'");
                on_response(std::move(err));
                return;
            }
        } else {
            if (!r.skip_value()) {
                std::string err;
                write_error_envelope(err, id, "skip failed");
                on_response(std::move(err));
                return;
            }
        }
    }
    if (!r.ok()) {
        std::string err;
        write_error_envelope(err, id, "parse error");
        on_response(std::move(err));
        return;
    }

    if (method_name.empty()) {
        std::string err;
        write_error_envelope(err, id, "missing 'method'");
        on_response(std::move(err));
        return;
    }

    for (auto& m : methods_) {
        if (m.name != method_name) continue;
        detail::json::reader args_r{args_view};
        if (m.async_invoke) {
            // Async path: the invoker captures id + on_response into a
            // respond callback. The method may fire respond inline (we
            // see the envelope before returning) or defer it — both
            // work the same from the caller's POV.
            //
            // Pass on_response by COPY (don't move) so we still have
            // our local copy to send an error envelope if args parsing
            // fails. async_invoke moves its parameter into the respond
            // lambda once args are valid.
            if (!m.async_invoke(this, args_r, id, on_response)) {
                std::string err;
                write_error_envelope(err, id, "args mismatch for '" + method_name + "'");
                on_response(std::move(err));
            }
            return;
        }
        if (m.invoke) {
            std::string result;
            if (!m.invoke(this, args_r, result)) {
                std::string err;
                write_error_envelope(err, id, "args mismatch for '" + method_name + "'");
                on_response(std::move(err));
                return;
            }
            std::string envelope;
            compose_result_envelope(envelope, id, result);
            on_response(std::move(envelope));
            return;
        }
        // method_entry with neither invoker — shouldn't happen
        std::string err;
        write_error_envelope(err, id, "invalid method entry: " + method_name);
        on_response(std::move(err));
        return;
    }

    std::string err;
    write_error_envelope(err, id, "unknown method: " + method_name);
    on_response(std::move(err));
}

} // namespace mpapp

#endif // MPAPP_HYBRID_BRIDGE_HPP
