// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0007 typed nested property-path
// accessor (mpapp::property_path) - the type-safe form of MAUI's string
// binding paths "A.B.C".

#include <memory>
#include <optional>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/binding/property_path.hpp>

using namespace mpapp;

namespace {

struct Leaf {
    int n = 0;
};

struct Mid {
    Leaf* leaf = nullptr;
};

struct Root {
    Mid* mid = nullptr;
};

// A second model using owning members + getter accessors, to exercise a
// 3-level path through value-returning lambdas.
struct Address {
    std::string city{};
};

struct Person {
    std::unique_ptr<Address> address;
};

struct Company {
    std::unique_ptr<Person> ceo;
};

} // namespace

// ---- single-level (Root -> Leaf) ------------------------------------------

TEST_CASE("property_path: single level read and write", "[mock][property_path]") {
    // Arrange
    Leaf leaf{ 7 };
    auto path = make_property_path(
        path_leaf([](Leaf& l) -> int* { return &l.n; }));

    // Act
    const std::optional<int> got = path.get(leaf);
    const bool               ok  = path.set(leaf, 99);

    // Assert
    REQUIRE(got.has_value());
    CHECK(*got == 7);
    CHECK(ok);
    CHECK(leaf.n == 99);
    CHECK(path.get(leaf) == std::optional<int>{ 99 });
}

// ---- two-level nesting (Root -> Mid -> Leaf) ------------------------------

TEST_CASE("property_path: two-level read through pointer chain",
          "[mock][property_path]") {
    // Arrange
    Leaf leaf{ 42 };
    Mid  mid{ &leaf };
    Root root{ &mid };

    auto path = make_property_path(
        path_link([](Root& r) { return r.mid; }),
        path_leaf([](Mid& m) -> int* { return m.leaf ? &m.leaf->n : nullptr; }));

    // Act
    const std::optional<int> got = path.get(root);

    // Assert
    REQUIRE(got.has_value());
    CHECK(*got == 42);
}

TEST_CASE("property_path: two-level write commits to the leaf",
          "[mock][property_path]") {
    // Arrange
    Leaf leaf{ 0 };
    Mid  mid{ &leaf };
    Root root{ &mid };

    auto path = make_property_path(
        path_link([](Root& r) { return r.mid; }),
        path_link([](Mid& m) { return m.leaf; }),
        path_leaf([](Leaf& l) -> int* { return &l.n; }));

    // Act
    const bool ok = path.set(root, 1234);

    // Assert
    CHECK(ok);
    CHECK(leaf.n == 1234);
    CHECK(path.get(root) == std::optional<int>{ 1234 });
}

// ---- three-level nesting through owning members ---------------------------

TEST_CASE("property_path: three-level read and write through unique_ptr chain",
          "[mock][property_path]") {
    // Arrange
    Company company{};
    company.ceo          = std::make_unique<Person>();
    company.ceo->address = std::make_unique<Address>();
    company.ceo->address->city = "Sofia";

    auto path = make_property_path(
        path_link([](Company& c) { return c.ceo.get(); }),
        path_link([](Person& p) { return p.address.get(); }),
        path_leaf([](Address& a) -> std::string* { return &a.city; }));

    // Act
    const std::optional<std::string> got = path.get(company);
    const bool                       ok  = path.set(company, std::string{ "Plovdiv" });

    // Assert
    REQUIRE(got.has_value());
    CHECK(*got == "Sofia");
    CHECK(ok);
    CHECK(company.ceo->address->city == "Plovdiv");
}

// ---- broken link: null intermediate ---------------------------------------

TEST_CASE("property_path: null intermediate yields nullopt on get",
          "[mock][property_path]") {
    // Arrange - mid is present, but its leaf pointer is null.
    Mid  mid{ nullptr };
    Root root{ &mid };

    auto path = make_property_path(
        path_link([](Root& r) { return r.mid; }),
        path_link([](Mid& m) { return m.leaf; }),
        path_leaf([](Leaf& l) -> int* { return &l.n; }));

    // Act / Assert
    CHECK_FALSE(path.get(root).has_value());
    CHECK(path.get(root) == std::nullopt);
}

TEST_CASE("property_path: null intermediate makes set a graceful no-op",
          "[mock][property_path]") {
    // Arrange - the very first link is null (root.mid == nullptr).
    Leaf leaf{ 5 };
    Root root{ nullptr };

    auto path = make_property_path(
        path_link([](Root& r) { return r.mid; }),
        path_link([](Mid& m) { return m.leaf; }),
        path_leaf([](Leaf& l) -> int* { return &l.n; }));

    // Act
    const bool ok = path.set(root, 9999);

    // Assert - set failed gracefully, untouched leaf, get is nullopt.
    CHECK_FALSE(ok);
    CHECK(leaf.n == 5);
    CHECK_FALSE(path.get(root).has_value());
}

TEST_CASE("property_path: terminal getter returning null yields nullopt",
          "[mock][property_path]") {
    // Arrange - chain is intact but the leaf accessor itself returns null.
    Mid  mid{ nullptr };
    Root root{ &mid };

    auto path = make_property_path(
        path_link([](Root& r) { return r.mid; }),
        path_leaf([](Mid& m) -> int* { return m.leaf ? &m.leaf->n : nullptr; }));

    // Act / Assert
    CHECK_FALSE(path.get(root).has_value());
    CHECK_FALSE(path.set(root, 1));
}

// ---- empty std::function / round-trip edge ---------------------------------

TEST_CASE("property_path: round-trip set then get on deep chain",
          "[mock][property_path]") {
    // Arrange
    Leaf leaf{ 1 };
    Mid  mid{ &leaf };
    Root root{ &mid };

    auto path = make_property_path(
        path_link([](Root& r) { return r.mid; }),
        path_link([](Mid& m) { return m.leaf; }),
        path_leaf([](Leaf& l) -> int* { return &l.n; }));

    // Act
    REQUIRE(path.set(root, 11));
    REQUIRE(path.set(root, 22));

    // Assert
    CHECK(path.get(root) == std::optional<int>{ 22 });
}
