// SPDX-License-Identifier: Apache-2.0
// Tests for mpapp::hybrid_bridge — the typed JSON-RPC bridge layer
// underneath HybridWebView per ADR-0018.

#include <functional>
#include <string>
#include <utility>
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

// ---- Phase F: async dispatch -----------------------------------------------

namespace {

class async_bridge : public mpapp::hybrid_bridge {
public:
    async_bridge() {
        // Sync method registered the normal way — for "sync goes
        // through async path" coverage.
        register_method("add_sync", &async_bridge::add_sync);

        // Async method that responds inline (calls respond before
        // returning) — for "async fires before dispatch_async
        // returns".
        register_async_method<int>("add_async_sync",
                                   &async_bridge::add_async_sync);

        // Async method that captures the respond callback for later
        // resolution.
        register_async_method<int>("defer_add",
                                   &async_bridge::defer_add);

        // Async method that returns a string.
        register_async_method<std::string>("defer_greet",
                                           &async_bridge::defer_greet);

        // Async method that intentionally calls respond twice — the
        // shared_ptr<bool> guard should drop the second.
        register_async_method<int>("double_respond",
                                   &async_bridge::double_respond);
    }

    int add_sync(int a, int b) { return a + b; }

    void add_async_sync(int a, int b, std::function<void(int)> respond) {
        respond(a + b);
    }

    void defer_add(int a, int b, std::function<void(int)> respond) {
        pending_int_a_       = a;
        pending_int_b_       = b;
        pending_int_respond_ = std::move(respond);
    }

    void defer_greet(const std::string& name,
                     std::function<void(std::string)> respond) {
        pending_str_name_    = name;
        pending_str_respond_ = std::move(respond);
    }

    void double_respond(int a, int b, std::function<void(int)> respond) {
        respond(a + b);
        respond(999);  // should be dropped by the fired-guard
    }

    void resolve_int()    { if (pending_int_respond_) pending_int_respond_(pending_int_a_ + pending_int_b_); }
    void resolve_string() { if (pending_str_respond_) pending_str_respond_("hi " + pending_str_name_); }

private:
    int                              pending_int_a_ = 0;
    int                              pending_int_b_ = 0;
    std::function<void(int)>         pending_int_respond_;
    std::string                      pending_str_name_;
    std::function<void(std::string)> pending_str_respond_;
};

} // namespace

TEST_CASE("dispatch_async fires inline for sync method",
          "[bridge][hybrid_bridge][async]") {
    async_bridge b;
    std::string  captured;
    bool         fired = false;
    b.dispatch_async(R"({"id":1,"method":"add_sync","args":[2,3]})",
                     [&](std::string r) { captured = std::move(r); fired = true; });
    CHECK(fired);
    CHECK(captured == R"({"id":1,"result":5})");
}

TEST_CASE("dispatch_async handles inline-responding async method",
          "[bridge][hybrid_bridge][async]") {
    async_bridge b;
    std::string  captured;
    bool         fired = false;
    b.dispatch_async(R"({"id":2,"method":"add_async_sync","args":[10,20]})",
                     [&](std::string r) { captured = std::move(r); fired = true; });
    CHECK(fired);
    CHECK(captured == R"({"id":2,"result":30})");
}

TEST_CASE("dispatch_async defers when method captures respond",
          "[bridge][hybrid_bridge][async]") {
    async_bridge b;
    std::string  captured;
    bool         fired = false;
    b.dispatch_async(R"({"id":3,"method":"defer_add","args":[7,8]})",
                     [&](std::string r) { captured = std::move(r); fired = true; });
    // Method captured the callback — no envelope yet.
    CHECK_FALSE(fired);
    CHECK(captured.empty());

    // Resolve later.
    b.resolve_int();
    CHECK(fired);
    CHECK(captured == R"({"id":3,"result":15})");
}

TEST_CASE("dispatch_async deferred string-result async method",
          "[bridge][hybrid_bridge][async]") {
    async_bridge b;
    std::string  captured;
    bool         fired = false;
    b.dispatch_async(R"({"id":4,"method":"defer_greet","args":["Ada"]})",
                     [&](std::string r) { captured = std::move(r); fired = true; });
    CHECK_FALSE(fired);
    b.resolve_string();
    CHECK(fired);
    CHECK(captured == R"({"id":4,"result":"hi Ada"})");
}

TEST_CASE("dispatch_async double-respond drops the second call",
          "[bridge][hybrid_bridge][async]") {
    async_bridge b;
    std::string  captured;
    int          fire_count = 0;
    b.dispatch_async(R"({"id":5,"method":"double_respond","args":[1,2]})",
                     [&](std::string r) { captured = std::move(r); ++fire_count; });
    CHECK(fire_count == 1);
    CHECK(captured == R"({"id":5,"result":3})");
}

TEST_CASE("dispatch_async handles unknown method with error envelope",
          "[bridge][hybrid_bridge][async]") {
    async_bridge b;
    std::string  captured;
    b.dispatch_async(R"({"id":6,"method":"missing","args":[]})",
                     [&](std::string r) { captured = std::move(r); });
    CHECK(captured == R"({"id":6,"error":"unknown method: missing"})");
}

TEST_CASE("dispatch_async handles malformed envelope with error",
          "[bridge][hybrid_bridge][async]") {
    async_bridge b;
    std::string  captured;
    b.dispatch_async("not json", [&](std::string r) { captured = std::move(r); });
    CHECK(captured == R"({"id":-1,"error":"malformed envelope"})");
}

TEST_CASE("dispatch_async on async method with bad args returns error",
          "[bridge][hybrid_bridge][async]") {
    async_bridge b;
    std::string  captured;
    b.dispatch_async(R"({"id":7,"method":"defer_add","args":["bad","args"]})",
                     [&](std::string r) { captured = std::move(r); });
    CHECK(captured == R"({"id":7,"error":"args mismatch for 'defer_add'"})");
}

TEST_CASE("sync dispatch on async-only method returns error",
          "[bridge][hybrid_bridge][async]") {
    async_bridge b;
    std::string  out;
    CHECK_FALSE(b.dispatch(R"({"id":8,"method":"defer_add","args":[1,2]})", out));
    CHECK(out == R"({"id":8,"error":"method is async: defer_add"})");
}

TEST_CASE("async bridge has both sync and async methods registered",
          "[bridge][hybrid_bridge][async]") {
    async_bridge b;
    auto names = b.method_names();
    REQUIRE(names.size() == 5);
    CHECK(names[0] == "add_sync");
    CHECK(names[1] == "add_async_sync");
    CHECK(names[2] == "defer_add");
    CHECK(names[3] == "defer_greet");
    CHECK(names[4] == "double_respond");
}
