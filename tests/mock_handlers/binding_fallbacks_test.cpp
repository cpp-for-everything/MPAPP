// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0007 binding fallbacks helpers
// (MAUI FallbackValue / TargetNullValue / StringFormat).

#include <optional>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/binding/binding_fallbacks.hpp>

using namespace mpapp;

TEST_CASE("apply_fallbacks returns source when present", "[mock][binding]") {
    // Arrange
    std::optional<int> source{ 42 };

    // Act
    const int result = apply_fallbacks(source, 7);

    // Assert
    CHECK(result == 42);
}

TEST_CASE("apply_fallbacks returns fallback when source missing",
          "[mock][binding]") {
    // Arrange
    std::optional<int> source{};

    // Act
    const int result = apply_fallbacks(source, 7);

    // Assert
    CHECK(result == 7);
}

TEST_CASE("apply_fallbacks works for string-typed values",
          "[mock][binding]") {
    CHECK(apply_fallbacks(std::optional<std::string>{ "hi" },
                          std::string{ "fb" }) == "hi");
    CHECK(apply_fallbacks(std::optional<std::string>{},
                          std::string{ "fb" }) == "fb");
}

TEST_CASE("apply_target_null returns source when present",
          "[mock][binding]") {
    // Arrange
    std::optional<std::string> source{ "value" };

    // Act
    const std::string result =
        apply_target_null(source, std::string{ "(null)" });

    // Assert
    CHECK(result == "value");
}

TEST_CASE("apply_target_null returns target-null when source absent",
          "[mock][binding]") {
    // Arrange
    std::optional<std::string> source{};

    // Act
    const std::string result =
        apply_target_null(source, std::string{ "(null)" });

    // Assert
    CHECK(result == "(null)");
}

TEST_CASE("string_format rewrites MAUI {0} placeholder", "[mock][binding]") {
    CHECK(string_format("{0}", 42) == "42");
    CHECK(string_format("value: {0}", std::string{ "x" }) == "value: x");
}

TEST_CASE("string_format applies a format spec after the index",
          "[mock][binding]") {
    CHECK(string_format("{0:.2f}", 3.5) == "3.50");
    CHECK(string_format("[{0:03}]", 7) == "[007]");
}

TEST_CASE("string_format preserves escaped braces", "[mock][binding]") {
    // "{{" and "}}" must survive as literal braces around the value.
    CHECK(string_format("{{{0}}}", 5) == "{5}");
    CHECK(string_format("{{literal}}", 0) == "{literal}");
}

TEST_CASE("string_format passes through a bare std::format placeholder",
          "[mock][binding]") {
    // A pattern with no positional index is left as a std::format "{}".
    CHECK(string_format("{}", 9) == "9");
    CHECK(string_format("n={:d}", 4) == "n=4");
}

TEST_CASE("binding_value::resolve uses fallback when source missing",
          "[mock][binding]") {
    // Arrange
    binding_value<int> resolver{};
    resolver.fallback = 99;

    // Act
    const std::string result = resolver.resolve(std::optional<int>{});

    // Assert
    CHECK(result == "99");
}

TEST_CASE("binding_value::resolve uses target_null when no fallback",
          "[mock][binding]") {
    // Arrange — only target_null configured.
    binding_value<std::string> resolver{};
    resolver.target_null = std::string{ "N/A" };

    // Act
    const std::string result =
        resolver.resolve(std::optional<std::string>{});

    // Assert
    CHECK(result == "N/A");
}

TEST_CASE("binding_value::resolve prefers fallback over target_null",
          "[mock][binding]") {
    // Arrange — both configured; FallbackValue wins for an unresolved source.
    binding_value<int> resolver{};
    resolver.fallback = 1;
    resolver.target_null = 2;

    // Act / Assert
    CHECK(resolver.resolve(std::optional<int>{}) == "1");
}

TEST_CASE("binding_value::resolve defaults when nothing configured",
          "[mock][binding]") {
    // Arrange
    binding_value<int> resolver{};

    // Act / Assert — default-constructed int -> "0".
    CHECK(resolver.resolve(std::optional<int>{}) == "0");
}

TEST_CASE("binding_value::resolve applies string_format to present value",
          "[mock][binding]") {
    // Arrange
    binding_value<double> resolver{};
    resolver.string_format = std::string{ "${0:.2f}" };

    // Act
    const std::string result = resolver.resolve(std::optional<double>{ 3.5 });

    // Assert
    CHECK(result == "$3.50");
}

TEST_CASE("binding_value::resolve stringifies present value without format",
          "[mock][binding]") {
    // Arrange — no string_format configured.
    binding_value<int> resolver{};

    // Act / Assert
    CHECK(resolver.resolve(std::optional<int>{ 123 }) == "123");
}

TEST_CASE("binding_value::resolve applies string_format to a fallback",
          "[mock][binding]") {
    // Arrange — unresolved source falls back, then renders through format.
    binding_value<int> resolver{};
    resolver.fallback = 8;
    resolver.string_format = std::string{ "[{0:02}]" };

    // Act / Assert
    CHECK(resolver.resolve(std::optional<int>{}) == "[08]");
}
