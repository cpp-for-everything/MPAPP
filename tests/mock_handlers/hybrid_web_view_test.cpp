// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::hybrid_web_view`.

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
    hybrid_web_view h;
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
    hybrid_web_view h;
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
    hybrid_web_view h;
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
    hybrid_web_view h;
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
    hybrid_web_view h;
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
    hybrid_web_view h;
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
    hybrid_web_view h;
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
    hybrid_web_view h;
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
    hybrid_web_view h;
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
