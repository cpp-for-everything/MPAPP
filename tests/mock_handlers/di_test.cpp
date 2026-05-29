// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0011 dependency-injection container.

#include <memory>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/di/app_builder.hpp>
#include <mpapp/di/service_collection.hpp>

using namespace mpapp;

namespace {

struct counter {
    int value = 0;
};

// An interface + impl for the interface->impl registration test.
struct greeter {
    virtual ~greeter() = default;
    virtual std::string hello() const = 0;
};
struct english_greeter : greeter {
    std::string hello() const override { return "hello"; }
};

// A service with a dependency, resolved via a factory (constructor
// injection, the C++ way).
struct repository {
    int rows = 3;
};
struct service {
    std::shared_ptr<repository> repo;
    explicit service(std::shared_ptr<repository> r) : repo{ std::move(r) } {}
    int row_count() const { return repo ? repo->rows : -1; }
};

} // namespace

TEST_CASE("singleton resolves to the same instance every time",
          "[mock][di]") {
    service_collection sc;
    sc.add_singleton<counter>();
    auto sp = sc.build();

    auto a = sp.get<counter>();
    auto b = sp.get<counter>();
    REQUIRE(a != nullptr);
    CHECK(a.get() == b.get());           // same instance

    a->value = 42;
    CHECK(sp.get<counter>()->value == 42); // shared state
}

TEST_CASE("transient resolves to a fresh instance every time",
          "[mock][di]") {
    service_collection sc;
    sc.add_transient<counter>();
    auto sp = sc.build();

    auto a = sp.get<counter>();
    auto b = sp.get<counter>();
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    CHECK(a.get() != b.get());           // distinct instances
}

TEST_CASE("interface -> implementation registration resolves the impl",
          "[mock][di]") {
    service_collection sc;
    sc.add_singleton<greeter, english_greeter>();
    auto sp = sc.build();

    auto g = sp.get<greeter>();
    REQUIRE(g != nullptr);
    CHECK(g->hello() == "hello");
}

TEST_CASE("factory registration performs constructor injection",
          "[mock][di]") {
    service_collection sc;
    sc.add_singleton<repository>();
    sc.add_singleton<service>(
        std::function<std::shared_ptr<service>(service_provider&)>{
            [](service_provider& sp) {
                return std::make_shared<service>(sp.get_required<repository>());
            } });
    auto sp = sc.build();

    auto svc = sp.get<service>();
    REQUIRE(svc != nullptr);
    CHECK(svc->row_count() == 3);
    // The injected repository is the same singleton instance.
    CHECK(svc->repo.get() == sp.get<repository>().get());
}

TEST_CASE("pre-built instance registration is returned as-is",
          "[mock][di]") {
    auto pre = std::make_shared<counter>();
    pre->value = 99;

    service_collection sc;
    sc.add_singleton<counter>(pre);
    auto sp = sc.build();

    CHECK(sp.get<counter>().get() == pre.get());
    CHECK(sp.get<counter>()->value == 99);
}

TEST_CASE("missing registrations: get returns null, get_required throws",
          "[mock][di]") {
    service_collection sc;
    auto sp = sc.build();

    CHECK(sp.get<counter>() == nullptr);
    CHECK_FALSE(sp.contains<counter>());
    CHECK_THROWS(sp.get_required<counter>());
}

TEST_CASE("app_builder exposes the service collection + builds a provider",
          "[mock][di][builder]") {
    app_builder builder;
    builder.services().add_singleton<counter>();
    auto sp = builder.build();

    auto c = sp.get<counter>();
    REQUIRE(c != nullptr);
    CHECK(c->value == 0);
}
