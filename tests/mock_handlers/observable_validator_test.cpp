// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for observable_validator (ObservableValidator /
// INotifyDataErrorInfo equivalent).

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/mvvm/observable_validator.hpp>

using namespace mpapp;

namespace {

// A small model with a required name + an age that must fall in [0, 130].
struct person {
    std::string name;
    int         age = 0;
};

// Attaches the standard rule set used across the tests below.
void attach_rules(observable_validator& v, const person& model) {
    v.add_rule("name", [&model]() -> std::optional<std::string> {
        if (model.name.empty()) {
            return std::string{ "name is required" };
        }
        return std::nullopt;
    });
    v.add_rule("age", [&model]() -> std::optional<std::string> {
        if (model.age < 0 || model.age > 130) {
            return std::string{ "age out of range" };
        }
        return std::nullopt;
    });
}

} // namespace

TEST_CASE("validate populates errors for an invalid model", "[mock][validator]") {
    // Arrange
    person              model{ "", -5 };
    observable_validator v;
    attach_rules(v, model);

    // Act
    const bool ok = v.validate();

    // Assert
    CHECK_FALSE(ok);
    CHECK(v.has_errors());
    CHECK(v.get_errors("name") == std::vector<std::string>{ "name is required" });
    CHECK(v.get_errors("age") == std::vector<std::string>{ "age out of range" });
    CHECK(v.get_all_errors().size() == 2);
}

TEST_CASE("fixing values + re-validate clears errors", "[mock][validator]") {
    // Arrange
    person              model{ "", -5 };
    observable_validator v;
    attach_rules(v, model);
    REQUIRE_FALSE(v.validate());
    REQUIRE(v.has_errors());

    // Act
    model.name = "Ada";
    model.age  = 36;
    const bool ok = v.validate();

    // Assert
    CHECK(ok);
    CHECK_FALSE(v.has_errors());
    CHECK(v.get_errors("name").empty());
    CHECK(v.get_errors("age").empty());
    CHECK(v.get_all_errors().empty());
}

TEST_CASE("has_errors transitions true then false", "[mock][validator]") {
    // Arrange
    person              model{ "", 200 };
    observable_validator v;
    attach_rules(v, model);

    // Act / Assert — starts clean (no validation run yet)
    CHECK_FALSE(v.has_errors());

    v.validate();
    CHECK(v.has_errors());

    model.name = "Grace";
    model.age  = 40;
    v.validate();
    CHECK_FALSE(v.has_errors());
}

TEST_CASE("errors_changed fires when error-set changes", "[mock][validator]") {
    // Arrange
    person              model{ "", 36 };
    observable_validator v;
    attach_rules(v, model);

    std::vector<std::string>     fired;
    signal_slot<std::string_view> slot;
    auto cb = [&](std::string_view property) { fired.emplace_back(property); };
    v.errors_changed.subscribe(slot, cb);

    // Act — name invalid, age valid: only "name" should fire.
    v.validate();

    // Assert
    CHECK(fired == std::vector<std::string>{ "name" });

    // Act — fix name: "name" cleared, fires again.
    fired.clear();
    model.name = "Linus";
    v.validate();

    // Assert
    CHECK(fired == std::vector<std::string>{ "name" });
}

TEST_CASE("errors_changed does not fire on idempotent re-validation",
          "[mock][validator]") {
    // Arrange
    person              model{ "", 36 };
    observable_validator v;
    attach_rules(v, model);
    v.validate(); // name now in error state

    int                           hits = 0;
    signal_slot<std::string_view> slot;
    auto cb = [&](std::string_view) { ++hits; };
    v.errors_changed.subscribe(slot, cb);

    // Act — same invalid state: no change, no emission.
    v.validate();

    // Assert
    CHECK(hits == 0);
    CHECK(v.has_errors());
}

TEST_CASE("validate_property runs only one property's rules",
          "[mock][validator]") {
    // Arrange
    person              model{ "", -1 };
    observable_validator v;
    attach_rules(v, model);

    // Act — validate only age.
    const bool age_ok = v.validate_property("age");

    // Assert — age recorded as invalid; name untouched (no rule run).
    CHECK_FALSE(age_ok);
    CHECK(v.get_errors("age") == std::vector<std::string>{ "age out of range" });
    CHECK(v.get_errors("name").empty());

    // Act — fix age, re-validate the single property.
    model.age          = 10;
    const bool age_ok2 = v.validate_property("age");

    // Assert
    CHECK(age_ok2);
    CHECK(v.get_errors("age").empty());
}

TEST_CASE("validate_property on an unregistered property is valid",
          "[mock][validator]") {
    // Arrange
    observable_validator v;

    // Act
    const bool ok = v.validate_property("does_not_exist");

    // Assert
    CHECK(ok);
    CHECK_FALSE(v.has_errors());
}

TEST_CASE("multiple rules on one property accumulate messages",
          "[mock][validator]") {
    // Arrange
    std::string          value;
    observable_validator v;
    v.add_rule("field", [&value]() -> std::optional<std::string> {
        return value.empty() ? std::optional<std::string>{ "must not be empty" }
                             : std::nullopt;
    });
    v.add_rule("field", [&value]() -> std::optional<std::string> {
        return value.size() < 3 ? std::optional<std::string>{ "too short" }
                                : std::nullopt;
    });

    // Act — empty value violates both rules.
    v.validate();

    // Assert
    CHECK(v.get_errors("field")
          == std::vector<std::string>{ "must not be empty", "too short" });
    CHECK(v.get_all_errors().size() == 2);
}

TEST_CASE("get_errors returns empty for unknown property", "[mock][validator]") {
    // Arrange
    observable_validator v;

    // Act / Assert
    CHECK(v.get_errors("unknown").empty());
    CHECK(v.get_all_errors().empty());
}

TEST_CASE("errors_changed fires when message set changes but stays non-empty",
          "[mock][validator]") {
    // Arrange — a rule whose message text depends on the model.
    int                  code = 1;
    observable_validator v;
    v.add_rule("status", [&code]() -> std::optional<std::string> {
        if (code == 0) {
            return std::nullopt;
        }
        return std::string{ "error " } + std::to_string(code);
    });

    std::vector<std::string>      fired;
    signal_slot<std::string_view> slot;
    auto cb = [&](std::string_view p) { fired.emplace_back(p); };
    v.errors_changed.subscribe(slot, cb);

    // Act — first failure.
    v.validate();
    CHECK(fired == std::vector<std::string>{ "status" });

    // Act — different message, still in error: should fire again.
    fired.clear();
    code = 2;
    v.validate();

    // Assert
    CHECK(fired == std::vector<std::string>{ "status" });
    CHECK(v.get_errors("status") == std::vector<std::string>{ "error 2" });
}
