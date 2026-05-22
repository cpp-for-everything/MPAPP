// SPDX-License-Identifier: Apache-2.0
// Tests for mpapp::hybrid_bridge — the typed JSON-RPC bridge layer
// underneath HybridWebView per ADR-0018.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/hybrid_bridge.hpp>

namespace {

class test_bridge : public mpapp::hybrid_bridge {
public:
    test_bridge() {
        register_method("add",       &test_bridge::add);
        register_method("greet",     &test_bridge::greet);
        register_method("toggle",    &test_bridge::toggle);
        register_method("sum_all",   &test_bridge::sum_all);
        register_method("notify",    &test_bridge::notify);
        register_method("ping",      &test_bridge::ping);
    }

    int         add(int a, int b)                          { return a + b; }
    std::string greet(const std::string& name)             { return "hi " + name; }
    bool        toggle(bool v)                             { return !v; }
    int         sum_all(const std::vector<int>& xs)        {
        int s = 0;
        for (int x : xs) s += x;
        return s;
    }
    void        notify(const std::string& msg)             { last_msg_ = msg; }
    std::string ping() const                                { return "pong"; }

    const std::string& last_msg() const noexcept { return last_msg_; }

private:
    std::string last_msg_;
};

} // namespace

TEST_CASE("bridge registers methods in order",
          "[bridge][hybrid_bridge]") {
    test_bridge b;
    auto names = b.method_names();
    REQUIRE(names.size() == 6);
    CHECK(names[0] == "add");
    CHECK(names[1] == "greet");
    CHECK(names[2] == "toggle");
    CHECK(names[3] == "sum_all");
    CHECK(names[4] == "notify");
    CHECK(names[5] == "ping");
}

TEST_CASE("bridge dispatches a single-arg method",
          "[bridge][hybrid_bridge]") {
    test_bridge b;
    std::string out;
    REQUIRE(b.dispatch(R"({"id":1,"method":"toggle","args":[true]})", out));
    CHECK(out == R"({"id":1,"result":false})");
}

TEST_CASE("bridge dispatches a two-arg method returning int",
          "[bridge][hybrid_bridge]") {
    test_bridge b;
    std::string out;
    REQUIRE(b.dispatch(R"({"id":7,"method":"add","args":[3,4]})", out));
    CHECK(out == R"({"id":7,"result":7})");
}

TEST_CASE("bridge dispatches a method returning a string",
          "[bridge][hybrid_bridge]") {
    test_bridge b;
    std::string out;
    REQUIRE(b.dispatch(R"({"id":2,"method":"greet","args":["Ada"]})", out));
    CHECK(out == R"({"id":2,"result":"hi Ada"})");
}

TEST_CASE("bridge dispatches a method taking vector<int>",
          "[bridge][hybrid_bridge]") {
    test_bridge b;
    std::string out;
    REQUIRE(b.dispatch(R"({"id":3,"method":"sum_all","args":[[1,2,3,4]]})", out));
    CHECK(out == R"({"id":3,"result":10})");
}

TEST_CASE("bridge dispatches a void method (returns null)",
          "[bridge][hybrid_bridge]") {
    test_bridge b;
    std::string out;
    REQUIRE(b.dispatch(R"({"id":9,"method":"notify","args":["hello"]})", out));
    CHECK(out == R"({"id":9,"result":null})");
    CHECK(b.last_msg() == "hello");
}

TEST_CASE("bridge dispatches a const method",
          "[bridge][hybrid_bridge]") {
    test_bridge b;
    std::string out;
    REQUIRE(b.dispatch(R"({"id":4,"method":"ping","args":[]})", out));
    CHECK(out == R"({"id":4,"result":"pong"})");
}

TEST_CASE("bridge handles unknown method with error envelope",
          "[bridge][hybrid_bridge]") {
    test_bridge b;
    std::string out;
    CHECK(!b.dispatch(R"({"id":5,"method":"nonexistent","args":[]})", out));
    CHECK(out == R"({"id":5,"error":"unknown method: nonexistent"})");
}

TEST_CASE("bridge handles malformed envelope with error",
          "[bridge][hybrid_bridge]") {
    test_bridge b;
    std::string out;
    CHECK(!b.dispatch("not a json object", out));
    CHECK(out == R"({"id":-1,"error":"malformed envelope"})");
}

TEST_CASE("bridge handles wrong-typed args with error",
          "[bridge][hybrid_bridge]") {
    test_bridge b;
    std::string out;
    CHECK(!b.dispatch(R"({"id":8,"method":"add","args":["not","ints"]})", out));
    CHECK(out == R"({"id":8,"error":"args mismatch for 'add'"})");
}

TEST_CASE("bridge handles wrong-arity args with error",
          "[bridge][hybrid_bridge]") {
    test_bridge b;
    std::string out;
    // add() needs 2 args; supplying 1.
    CHECK(!b.dispatch(R"({"id":11,"method":"add","args":[3]})", out));
    CHECK(out == R"({"id":11,"error":"args mismatch for 'add'"})");
}

TEST_CASE("bridge tolerates field order in envelope",
          "[bridge][hybrid_bridge]") {
    test_bridge b;
    std::string out;
    // method first, id last
    REQUIRE(b.dispatch(R"({"method":"add","args":[5,6],"id":12})", out));
    CHECK(out == R"({"id":12,"result":11})");
}
