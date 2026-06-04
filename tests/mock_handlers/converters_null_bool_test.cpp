// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the CTK null/bool converters (batch 1).

#include <optional>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/binding/converters_null_bool.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// is_null - std::optional
// ---------------------------------------------------------------------------

TEST_CASE("is_null_converter: empty optional -> true", "[mock][converters_null_bool]") {
    is_null_converter<int> c;

    // Arrange
    std::optional<int> empty{};
    std::optional<int> full{ 42 };

    // Act + Assert
    CHECK(c.convert(empty) == true);
    CHECK(c.convert(full)  == false);
    CHECK(c.convert_back(true) == std::nullopt);
    CHECK(c.convert_back(false) == std::nullopt);
}

TEST_CASE("is_null free function: empty optional -> true", "[mock][converters_null_bool]") {
    auto fn = is_null<int>();

    CHECK(fn(std::nullopt) == true);
    CHECK(fn(std::optional<int>{ 0 }) == false);
    CHECK(fn(std::optional<int>{ -1 }) == false);
}

// ---------------------------------------------------------------------------
// is_null - raw pointer
// ---------------------------------------------------------------------------

TEST_CASE("is_null_ptr_converter: null pointer -> true", "[mock][converters_null_bool]") {
    is_null_ptr_converter<int> c;

    int val = 7;
    int* non_null = &val;
    int* null_ptr = nullptr;

    CHECK(c.convert(null_ptr)  == true);
    CHECK(c.convert(non_null)  == false);
    CHECK(c.convert_back(true)  == nullptr);
    CHECK(c.convert_back(false) == nullptr);
}

TEST_CASE("is_null_ptr free function: null pointer -> true", "[mock][converters_null_bool]") {
    auto fn = is_null_ptr<int>();

    int x = 3;
    CHECK(fn(nullptr) == true);
    CHECK(fn(&x)      == false);
}

// ---------------------------------------------------------------------------
// is_not_null - std::optional
// ---------------------------------------------------------------------------

TEST_CASE("is_not_null_converter: has_value -> true", "[mock][converters_null_bool]") {
    is_not_null_converter<std::string> c;

    std::optional<std::string> empty{};
    std::optional<std::string> full{ "hello" };

    CHECK(c.convert(empty) == false);
    CHECK(c.convert(full)  == true);
    CHECK(c.convert_back(true)  == std::nullopt);
    CHECK(c.convert_back(false) == std::nullopt);
}

TEST_CASE("is_not_null free function: has_value -> true", "[mock][converters_null_bool]") {
    auto fn = is_not_null<double>();

    CHECK(fn(std::nullopt)              == false);
    CHECK(fn(std::optional<double>{ 0.0 }) == true);
    CHECK(fn(std::optional<double>{ 3.14 }) == true);
}

// ---------------------------------------------------------------------------
// is_not_null - raw pointer
// ---------------------------------------------------------------------------

TEST_CASE("is_not_null_ptr_converter: non-null pointer -> true", "[mock][converters_null_bool]") {
    is_not_null_ptr_converter<double> c;

    double d = 1.5;
    double* p = &d;
    double* np = nullptr;

    CHECK(c.convert(p)   == true);
    CHECK(c.convert(np)  == false);
    CHECK(c.convert_back(true)  == nullptr);
    CHECK(c.convert_back(false) == nullptr);
}

TEST_CASE("is_not_null_ptr free function: non-null -> true", "[mock][converters_null_bool]") {
    auto fn = is_not_null_ptr<int>();

    int v = 99;
    CHECK(fn(&v)      == true);
    CHECK(fn(nullptr) == false);
}

// ---------------------------------------------------------------------------
// is_not_null_or_empty - std::string
// ---------------------------------------------------------------------------

TEST_CASE("is_not_null_or_empty_converter (string): non-empty -> true",
          "[mock][converters_null_bool]") {
    is_not_null_or_empty_converter c;

    CHECK(c.convert("")        == false);
    CHECK(c.convert("a")       == true);
    CHECK(c.convert("hello")   == true);
    CHECK(c.convert_back(true)  == "");
    CHECK(c.convert_back(false) == "");
}

TEST_CASE("is_not_null_or_empty free function (string)",
          "[mock][converters_null_bool]") {
    auto fn = is_not_null_or_empty();

    CHECK(fn("")       == false);
    CHECK(fn("x")      == true);
    CHECK(fn("  ")     == true);   // spaces are not empty
}

// ---------------------------------------------------------------------------
// is_not_null_or_empty - std::optional<std::string>
// ---------------------------------------------------------------------------

TEST_CASE("is_not_null_or_empty_opt_converter: nullopt/empty -> false",
          "[mock][converters_null_bool]") {
    is_not_null_or_empty_opt_converter c;

    CHECK(c.convert(std::nullopt)                       == false);
    CHECK(c.convert(std::optional<std::string>{ "" })   == false);
    CHECK(c.convert(std::optional<std::string>{ "hi" }) == true);
    CHECK(c.convert_back(true)  == std::nullopt);
    CHECK(c.convert_back(false) == std::nullopt);
}

TEST_CASE("is_not_null_or_empty_opt free function",
          "[mock][converters_null_bool]") {
    auto fn = is_not_null_or_empty_opt();

    CHECK(fn(std::nullopt)                        == false);
    CHECK(fn(std::optional<std::string>{ "" })    == false);
    CHECK(fn(std::optional<std::string>{ "ok" })  == true);
}

// ---------------------------------------------------------------------------
// is_string_not_null_or_whitespace - std::string
// ---------------------------------------------------------------------------

TEST_CASE("is_string_not_null_or_whitespace_converter: whitespace-only -> false",
          "[mock][converters_null_bool]") {
    is_string_not_null_or_whitespace_converter c;

    CHECK(c.convert("")        == false);
    CHECK(c.convert(" ")       == false);
    CHECK(c.convert("\t\n\r ") == false);
    CHECK(c.convert("a")       == true);
    CHECK(c.convert("  x ")    == true);
    CHECK(c.convert_back(true)  == "");
    CHECK(c.convert_back(false) == "");
}

TEST_CASE("is_string_not_null_or_whitespace free function",
          "[mock][converters_null_bool]") {
    auto fn = is_string_not_null_or_whitespace();

    CHECK(fn("")        == false);
    CHECK(fn("   ")     == false);
    CHECK(fn("\t")      == false);
    CHECK(fn("hello")   == true);
    CHECK(fn("  hi  ")  == true);
}

// ---------------------------------------------------------------------------
// is_string_not_null_or_whitespace - std::optional<std::string>
// ---------------------------------------------------------------------------

TEST_CASE("is_string_not_null_or_whitespace_opt_converter: nullopt/blank -> false",
          "[mock][converters_null_bool]") {
    is_string_not_null_or_whitespace_opt_converter c;

    CHECK(c.convert(std::nullopt)                        == false);
    CHECK(c.convert(std::optional<std::string>{ "" })    == false);
    CHECK(c.convert(std::optional<std::string>{ "  " })  == false);
    CHECK(c.convert(std::optional<std::string>{ "hi" })  == true);
    CHECK(c.convert(std::optional<std::string>{ " x " }) == true);
    CHECK(c.convert_back(true)  == std::nullopt);
    CHECK(c.convert_back(false) == std::nullopt);
}

TEST_CASE("is_string_not_null_or_whitespace_opt free function",
          "[mock][converters_null_bool]") {
    auto fn = is_string_not_null_or_whitespace_opt();

    CHECK(fn(std::nullopt)                         == false);
    CHECK(fn(std::optional<std::string>{ "\n\t " }) == false);
    CHECK(fn(std::optional<std::string>{ "ok" })    == true);
}
