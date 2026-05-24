// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_hybrid_web_view`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/hybrid_web_view_handler.hpp>
#include <mpapp/hybrid_bridge.hpp>
#include <mpapp/hybrid_web_view.hpp>

using namespace mpapp;

namespace {

// Test bridge used by the bridge-integration cases below.
class echo_bridge : public hybrid_bridge {
public:
    echo_bridge() {
        register_method("add",   &echo_bridge::add);
        register_method("greet", &echo_bridge::greet);
    }
    int         add(int a, int b)              { return a + b; }
    std::string greet(const std::string& name) { return "hi " + name; }
};

} // namespace

TEST_CASE("hybrid_web_view bridge sends and receives",
          "[mock][hybrid_web_view]") {
    internal::basic_hybrid_web_view h;
    CHECK(h.hybrid_namespace.get() == "mpapp");
    CHECK(h.last_message_in.get().empty());
    CHECK(h.last_message_out().empty());

    h.send_to_js("from-cpp");
    CHECK(h.last_message_out() == "from-cpp");

    h.simulate_inbound("from-js");
    CHECK(h.last_message_in.get() == "from-js");
}

TEST_CASE("mock handler records bridge traffic",
          "[mock][hybrid_web_view]") {
    internal::basic_hybrid_web_view h;
    hybrid_web_view_handler<platform::mock> hh;
    hh.map_messages(h);
    hh.clear_calls();

    h.simulate_inbound("hello-from-js");
    h.send_to_js("hello-from-cpp");

    REQUIRE(hh.calls_as_strings() == std::vector<std::string>{
        "message_received=hello-from-js",
        "message_sent=hello-from-cpp",
    });
}

TEST_CASE("set_bridge<T> attaches a typed bridge",
          "[mock][hybrid_web_view][bridge]") {
    internal::basic_hybrid_web_view h;
    CHECK(!h.has_bridge());
    CHECK(h.bridge() == nullptr);

    echo_bridge& b = h.set_bridge<echo_bridge>();
    CHECK(h.has_bridge());
    CHECK(h.bridge() == &b);
    // Bridge has the methods we registered.
    auto names = b.method_names();
    REQUIRE(names.size() == 2);
    CHECK(names[0] == "add");
    CHECK(names[1] == "greet");
}

TEST_CASE("attached bridge handles JSON envelope and posts response via send_to_js",
          "[mock][hybrid_web_view][bridge]") {
    internal::basic_hybrid_web_view h;
    h.set_bridge<echo_bridge>();

    // Capture outbound traffic.
    std::vector<std::string> outbound;
    struct cb_t {
        std::vector<std::string>* out;
        void operator()(const std::string& v) const { out->push_back(v); }
    };
    cb_t cb{&outbound};
    signal_slot<const std::string&> slot{};
    h.message_sent.subscribe(slot, cb);

    h.simulate_inbound(R"({"id":1,"method":"add","args":[3,4]})");

    REQUIRE(outbound.size() == 1);
    CHECK(outbound[0] == R"({"id":1,"result":7})");
    // The envelope is in last_message_in for debug observers...
    CHECK(h.last_message_in.get() == R"({"id":1,"method":"add","args":[3,4]})");
    // ...and message_sent's last value matches.
    CHECK(h.last_message_out() == R"({"id":1,"result":7})");
}

TEST_CASE("attached bridge does NOT fire message_received for envelopes",
          "[mock][hybrid_web_view][bridge]") {
    internal::basic_hybrid_web_view h;
    h.set_bridge<echo_bridge>();

    int received_hits = 0;
    struct cb_t {
        int* hits;
        void operator()(const std::string&) const { ++*hits; }
    };
    cb_t cb{&received_hits};
    signal_slot<const std::string&> slot{};
    h.message_received.subscribe(slot, cb);

    // Envelope — bridge consumes; message_received should NOT fire.
    h.simulate_inbound(R"({"id":2,"method":"greet","args":["Ada"]})");
    CHECK(received_hits == 0);

    // Non-envelope — bridge ignores; message_received fires.
    h.simulate_inbound("raw plain string");
    CHECK(received_hits == 1);
}

TEST_CASE("attached bridge unknown method writes error envelope",
          "[mock][hybrid_web_view][bridge]") {
    internal::basic_hybrid_web_view h;
    h.set_bridge<echo_bridge>();

    std::string last_response;
    struct cb_t {
        std::string* dst;
        void operator()(const std::string& v) const { *dst = v; }
    };
    cb_t cb{&last_response};
    signal_slot<const std::string&> slot{};
    h.message_sent.subscribe(slot, cb);

    h.simulate_inbound(R"({"id":9,"method":"missing","args":[]})");
    CHECK(last_response == R"({"id":9,"error":"unknown method: missing"})");
}

TEST_CASE("no bridge attached: envelopes still go through message_received",
          "[mock][hybrid_web_view][bridge]") {
    // Bridge is opt-in. Without one, even JSON-shaped payloads
    // surface on message_received as raw strings.
    internal::basic_hybrid_web_view h;
    std::string received;
    struct cb_t {
        std::string* dst;
        void operator()(const std::string& v) const { *dst = v; }
    };
    cb_t cb{&received};
    signal_slot<const std::string&> slot{};
    h.message_received.subscribe(slot, cb);

    h.simulate_inbound(R"({"id":3,"method":"add","args":[1,2]})");
    CHECK(received == R"({"id":3,"method":"add","args":[1,2]})");
}

TEST_CASE("invoke_js writes a typed JSON-RPC envelope through send_to_js",
          "[mock][hybrid_web_view][bridge]") {
    internal::basic_hybrid_web_view h;
    std::vector<std::string> outbound;
    struct cb_t {
        std::vector<std::string>* out;
        void operator()(const std::string& v) const { out->push_back(v); }
    };
    cb_t cb{&outbound};
    signal_slot<const std::string&> slot{};
    h.message_sent.subscribe(slot, cb);

    const int id1 = h.invoke_js("setUser", std::string{"Ada"}, 42);
    const int id2 = h.invoke_js("ping");
    const int id3 = h.invoke_js("addItems", std::vector<int>{1, 2, 3}, true);

    REQUIRE(outbound.size() == 3);
    CHECK(outbound[0] == R"({"id":1,"method":"setUser","args":["Ada",42]})");
    CHECK(outbound[1] == R"({"id":2,"method":"ping","args":[]})");
    CHECK(outbound[2] == R"({"id":3,"method":"addItems","args":[[1,2,3],true]})");

    // The returned id matches what the envelope reports — caller can
    // correlate responses by it.
    CHECK(id1 == 1);
    CHECK(id2 == 2);
    CHECK(id3 == 3);

    // last_message_out keeps the most recent envelope.
    CHECK(h.last_message_out() == outbound[2]);
}

TEST_CASE("invoke_js + bridge dispatch interleave on message_sent",
          "[mock][hybrid_web_view][bridge]") {
    // invoke_js increments the outbound id counter; bridge responses
    // use the inbound envelope's id. Verify they don't collide and
    // that they can interleave on message_sent.
    internal::basic_hybrid_web_view h;
    h.set_bridge<echo_bridge>();

    std::vector<std::string> outbound;
    struct cb_t {
        std::vector<std::string>* out;
        void operator()(const std::string& v) const { out->push_back(v); }
    };
    cb_t cb{&outbound};
    signal_slot<const std::string&> slot{};
    h.message_sent.subscribe(slot, cb);

    // Outbound: id=1
    h.invoke_js("notify", std::string{"start"});
    // Inbound bridge call with id=99 — response uses id=99
    h.simulate_inbound(R"({"id":99,"method":"add","args":[2,3]})");
    // Outbound: id=2
    h.invoke_js("notify", std::string{"done"});

    REQUIRE(outbound.size() == 3);
    CHECK(outbound[0] == R"({"id":1,"method":"notify","args":["start"]})");
    CHECK(outbound[1] == R"({"id":99,"result":5})");
    CHECK(outbound[2] == R"({"id":2,"method":"notify","args":["done"]})");
}

TEST_CASE("invoke_js_cb fires the callback when JS posts a response",
          "[mock][hybrid_web_view][bridge]") {
    internal::basic_hybrid_web_view h;

    std::optional<int> received_result;
    int                hits = 0;
    auto cb = [&](std::optional<int> r) {
        received_result = r;
        ++hits;
    };

    const int id = h.invoke_js_cb<int>("add", cb, 3, 4);
    CHECK(id == 1);
    CHECK(h.pending_response_count() == 1);
    CHECK(h.last_message_out() == R"({"id":1,"method":"add","args":[3,4]})");

    // JS posts the response.
    h.simulate_inbound(R"({"id":1,"result":7})");

    CHECK(hits == 1);
    REQUIRE(received_result.has_value());
    CHECK(*received_result == 7);
    // Map was cleaned up after dispatch.
    CHECK(h.pending_response_count() == 0);
}

TEST_CASE("invoke_js_cb fires with nullopt on error response",
          "[mock][hybrid_web_view][bridge]") {
    internal::basic_hybrid_web_view h;
    std::optional<std::string> received;
    int                        hits = 0;
    auto cb = [&](std::optional<std::string> r) {
        received = std::move(r);
        ++hits;
    };

    h.invoke_js_cb<std::string>("greet", cb, std::string{"Ada"});
    h.simulate_inbound(R"({"id":1,"error":"unknown method: greet"})");

    CHECK(hits == 1);
    CHECK(!received.has_value());
    CHECK(h.pending_response_count() == 0);
}

TEST_CASE("invoke_js_cb pending callbacks route to the right id",
          "[mock][hybrid_web_view][bridge]") {
    internal::basic_hybrid_web_view h;

    std::vector<std::pair<int, std::optional<int>>> got;
    auto make_cb = [&](int label) {
        return [&got, label](std::optional<int> r) { got.emplace_back(label, r); };
    };

    h.invoke_js_cb<int>("a", make_cb(100), 1);   // id=1
    h.invoke_js_cb<int>("b", make_cb(200), 2);   // id=2
    h.invoke_js_cb<int>("c", make_cb(300), 3);   // id=3
    CHECK(h.pending_response_count() == 3);

    // Responses arrive out of order.
    h.simulate_inbound(R"({"id":2,"result":222})");
    h.simulate_inbound(R"({"id":3,"result":333})");
    h.simulate_inbound(R"({"id":1,"result":111})");

    REQUIRE(got.size() == 3);
    CHECK(got[0].first == 200); REQUIRE(got[0].second.has_value()); CHECK(*got[0].second == 222);
    CHECK(got[1].first == 300); REQUIRE(got[1].second.has_value()); CHECK(*got[1].second == 333);
    CHECK(got[2].first == 100); REQUIRE(got[2].second.has_value()); CHECK(*got[2].second == 111);
    CHECK(h.pending_response_count() == 0);
}

TEST_CASE("response with no matching pending callback falls through to message_received",
          "[mock][hybrid_web_view][bridge]") {
    internal::basic_hybrid_web_view h;
    std::string received;
    struct rb_t {
        std::string* dst;
        void operator()(const std::string& v) const { *dst = v; }
    };
    rb_t cb{&received};
    signal_slot<const std::string&> slot{};
    h.message_received.subscribe(slot, cb);

    // No callback registered for id=42. The envelope just looks like
    // raw inbound traffic to the listener.
    h.simulate_inbound(R"({"id":42,"result":"stale"})");
    CHECK(received == R"({"id":42,"result":"stale"})");
}

#include <mpapp/executor.hpp>

namespace {

// Driver coroutine for the Phase E task<T> async-bridge tests. Stores
// the result so the test can inspect it after simulate_inbound resumes
// the coroutine.
mpapp::task<void> drive_invoke_js_async(internal::basic_hybrid_web_view& wv,
                                        std::optional<int>& out) {
    out = co_await wv.invoke_js_async<int>("add", 3, 4);
    co_return;
}

mpapp::task<void> drive_invoke_js_async_error(internal::basic_hybrid_web_view& wv,
                                              std::optional<std::string>& out) {
    out = co_await wv.invoke_js_async<std::string>("greet", std::string{"Ada"});
    co_return;
}

} // namespace

TEST_CASE("invoke_js_async resolves the coroutine when JS posts a response",
          "[mock][hybrid_web_view][bridge][async]") {
    internal::basic_hybrid_web_view h;
    std::optional<int> received;

    // Eager-start coroutine — runs until the first co_await suspends.
    auto t = drive_invoke_js_async(h, received);

    CHECK(h.pending_response_count() == 1);
    CHECK(!t.is_ready());
    CHECK(h.last_message_out() == R"({"id":1,"method":"add","args":[3,4]})");

    // Drive the response. invoke_js_cb's stored callback fires inline
    // from process_inbound, which resumes the coroutine.
    h.simulate_inbound(R"({"id":1,"result":7})");

    CHECK(t.is_ready());
    REQUIRE(received.has_value());
    CHECK(*received == 7);
    CHECK(h.pending_response_count() == 0);
}

TEST_CASE("invoke_js_async error response resolves the coroutine to nullopt",
          "[mock][hybrid_web_view][bridge][async]") {
    internal::basic_hybrid_web_view h;
    std::optional<std::string> received;
    auto t = drive_invoke_js_async_error(h, received);

    CHECK(h.pending_response_count() == 1);
    h.simulate_inbound(R"({"id":1,"error":"unknown method: greet"})");

    CHECK(t.is_ready());
    CHECK(!received.has_value());
    CHECK(h.pending_response_count() == 0);
}

// ---- Phase F: async-method bridge through process_inbound ------------------

namespace {

class deferred_bridge : public hybrid_bridge {
public:
    deferred_bridge() {
        register_async_method<int>("compute", &deferred_bridge::compute);
    }
    void compute(int a, int b, std::function<void(int)> respond) {
        a_       = a;
        b_       = b;
        respond_ = std::move(respond);
    }
    void resolve() {
        if (respond_) respond_(a_ + b_);
    }
private:
    int                      a_{};
    int                      b_{};
    std::function<void(int)> respond_;
};

} // namespace

TEST_CASE("bridge async method defers response across process_inbound",
          "[mock][hybrid_web_view][bridge][async]") {
    internal::basic_hybrid_web_view h;
    auto& b = h.set_bridge<deferred_bridge>();

    std::vector<std::string> outbound;
    struct cb_t {
        std::vector<std::string>* out;
        void operator()(const std::string& v) const { out->push_back(v); }
    };
    cb_t cb{&outbound};
    signal_slot<const std::string&> slot{};
    h.message_sent.subscribe(slot, cb);

    h.simulate_inbound(R"({"id":42,"method":"compute","args":[10,5]})");
    // Async method captured the respond callback; nothing on the wire
    // yet.
    CHECK(outbound.empty());

    // Resolve later — the same id should appear on the bridge response.
    b.resolve();
    REQUIRE(outbound.size() == 1);
    CHECK(outbound[0] == R"({"id":42,"result":15})");
}

TEST_CASE("bridge async method bad-args still posts error inline",
          "[mock][hybrid_web_view][bridge][async]") {
    internal::basic_hybrid_web_view h;
    h.set_bridge<deferred_bridge>();

    std::string last_response;
    struct cb_t {
        std::string* dst;
        void operator()(const std::string& v) const { *dst = v; }
    };
    cb_t cb{&last_response};
    signal_slot<const std::string&> slot{};
    h.message_sent.subscribe(slot, cb);

    h.simulate_inbound(R"({"id":3,"method":"compute","args":["bad","args"]})");
    CHECK(last_response == R"({"id":3,"error":"args mismatch for 'compute'"})");
}
