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

#include <functional>
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

    // Dispatch an inbound JSON-RPC envelope. `payload` is the raw
    // envelope text; `out` receives the response envelope (success or
    // error). Returns true if the envelope was structurally valid and
    // a method was found and invoked; false if the envelope was
    // malformed or the method is unknown.
    //
    // Either way `out` is populated with a well-formed JSON envelope
    // suitable for posting back to JS — even on error.
    bool dispatch(std::string_view payload, std::string& out);

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
            make_invoker<Self, Ret, Args...>(method)
        });
    }

    // Const-member overload.
    template <class Self, class Ret, class... Args>
    void register_method(std::string name, Ret (Self::*method)(Args...) const) {
        methods_.push_back(method_entry{
            std::move(name),
            make_invoker_const<Self, Ret, Args...>(method)
        });
    }

private:
    struct method_entry {
        std::string                                              name;
        // Reads args from the reader (expects '[' ... ']'), invokes
        // the method on the bridge, writes result into out. Returns
        // false if args couldn't be parsed.
        std::function<bool(hybrid_bridge*, detail::json::reader&, std::string& /*result_out*/)> invoke;
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
        detail::json::reader args_r{args_view};
        std::string result;
        if (!m.invoke(this, args_r, result)) {
            write_error_envelope(out, id, "args mismatch for '" + method_name + "'");
            return false;
        }
        // Compose the response envelope. `result` is already JSON text
        // (whatever the method's return-value `write` produced); we
        // splice it in unquoted as the value of "result".
        out.clear();
        out.reserve(32 + result.size());
        out += "{\"id\":";
        {
            char buf[32];
            const int n = std::snprintf(buf, sizeof(buf), "%d", id);
            if (n > 0) out.append(buf, static_cast<std::size_t>(n));
        }
        out += ",\"result\":";
        out += result;
        out += '}';
        return true;
    }

    write_error_envelope(out, id, "unknown method: " + method_name);
    return false;
}

} // namespace mpapp

#endif // MPAPP_HYBRID_BRIDGE_HPP
