// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for multi_value_converter - all_true, any_true,
// concat, and chain_converter (two-stage composition).

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/binding/multi_value_converter.hpp>

using namespace mpapp;

// ===========================================================================
// all_true_converter
// ===========================================================================

TEST_CASE("all_true_converter: all true -> true", "[mock][multi_value_converter]") {
    // Arrange
    all_true_converter c;
    std::vector<bool>  values{ true, true, true };

    // Act
    bool result = c.convert(values);

    // Assert
    CHECK(result == true);
}

TEST_CASE("all_true_converter: one false -> false", "[mock][multi_value_converter]") {
    // Arrange
    all_true_converter c;
    std::vector<bool>  values{ true, false, true };

    // Act + Assert
    CHECK(c.convert(values) == false);
}

TEST_CASE("all_true_converter: all false -> false", "[mock][multi_value_converter]") {
    all_true_converter c;
    CHECK(c.convert({ false, false }) == false);
}

TEST_CASE("all_true_converter: empty vector -> true (vacuous conjunction)",
          "[mock][multi_value_converter]") {
    all_true_converter c;
    CHECK(c.convert({}) == true);
}

TEST_CASE("all_true_converter: single true -> true", "[mock][multi_value_converter]") {
    all_true_converter c;
    CHECK(c.convert({ true }) == true);
}

TEST_CASE("all_true_converter: single false -> false", "[mock][multi_value_converter]") {
    all_true_converter c;
    CHECK(c.convert({ false }) == false);
}

// Free-function helper mirrors the converter object behaviour.
TEST_CASE("all_true() free function matches converter results",
          "[mock][multi_value_converter]") {
    auto fn = all_true();
    CHECK(fn({ true, true })         == true);
    CHECK(fn({ true, false })        == false);
    CHECK(fn({})                     == true);   // empty -> vacuously true
}

// ===========================================================================
// any_true_converter
// ===========================================================================

TEST_CASE("any_true_converter: at least one true -> true", "[mock][multi_value_converter]") {
    // Arrange
    any_true_converter c;
    std::vector<bool>  values{ false, true, false };

    // Act + Assert
    CHECK(c.convert(values) == true);
}

TEST_CASE("any_true_converter: all false -> false", "[mock][multi_value_converter]") {
    any_true_converter c;
    CHECK(c.convert({ false, false, false }) == false);
}

TEST_CASE("any_true_converter: all true -> true", "[mock][multi_value_converter]") {
    any_true_converter c;
    CHECK(c.convert({ true, true }) == true);
}

TEST_CASE("any_true_converter: empty vector -> false (vacuous disjunction)",
          "[mock][multi_value_converter]") {
    any_true_converter c;
    CHECK(c.convert({}) == false);
}

TEST_CASE("any_true_converter: single true -> true", "[mock][multi_value_converter]") {
    any_true_converter c;
    CHECK(c.convert({ true }) == true);
}

TEST_CASE("any_true_converter: single false -> false", "[mock][multi_value_converter]") {
    any_true_converter c;
    CHECK(c.convert({ false }) == false);
}

TEST_CASE("any_true() free function matches converter results",
          "[mock][multi_value_converter]") {
    auto fn = any_true();
    CHECK(fn({ false, true })  == true);
    CHECK(fn({ false, false }) == false);
    CHECK(fn({})               == false);  // empty -> vacuously false
}

// ===========================================================================
// concat_converter
// ===========================================================================

TEST_CASE("concat_converter: joins strings without separator by default",
          "[mock][multi_value_converter]") {
    concat_converter c;
    CHECK(c.convert({ "Hello", " ", "World" }) == "Hello World");
}

TEST_CASE("concat_converter: joins strings with a separator",
          "[mock][multi_value_converter]") {
    concat_converter c{ ", " };
    CHECK(c.convert({ "one", "two", "three" }) == "one, two, three");
}

TEST_CASE("concat_converter: single element produces that element",
          "[mock][multi_value_converter]") {
    concat_converter c{ "-" };
    CHECK(c.convert({ "only" }) == "only");
}

TEST_CASE("concat_converter: empty vector produces empty string",
          "[mock][multi_value_converter]") {
    concat_converter c{ "," };
    CHECK(c.convert({}) == "");
}

TEST_CASE("concat_strings() free function matches converter results",
          "[mock][multi_value_converter]") {
    auto fn = concat_strings(" | ");
    CHECK(fn({ "a", "b", "c" }) == "a | b | c");
    CHECK(fn({})                 == "");
    CHECK(fn({ "solo" })         == "solo");
}

// ===========================================================================
// chain_converter - two-stage composition
// ===========================================================================

TEST_CASE("chain_converter: int -> string -> string (length label)",
          "[mock][multi_value_converter]") {
    // Stage 1: int -> std::string via std::to_string
    std::function<std::string(const int&)> to_str =
        [](const int& n) { return std::to_string(n); };

    // Stage 2: std::string -> std::string - wrap in brackets
    std::function<std::string(const std::string&)> bracket =
        [](const std::string& s) { return "[" + s + "]"; };

    auto chain = chain_converter<int, std::string, std::string>(to_str, bracket);

    // Arrange + Act + Assert
    CHECK(chain(42)   == "[42]");
    CHECK(chain(0)    == "[0]");
    CHECK(chain(-7)   == "[-7]");
}

TEST_CASE("chain_converter: bool -> int -> string (invert then format)",
          "[mock][multi_value_converter]") {
    // Stage 1: bool -> int (invert: true->0, false->1)
    std::function<int(const bool&)> to_int =
        [](const bool& b) { return b ? 0 : 1; };

    // Stage 2: int -> string
    std::function<std::string(const int&)> to_str =
        [](const int& n) { return n == 0 ? "off" : "on"; };

    auto chain = chain_converter<bool, int, std::string>(to_int, to_str);

    CHECK(chain(true)  == "off");   // true -> 0 -> "off"
    CHECK(chain(false) == "on");    // false -> 1 -> "on"
}

TEST_CASE("chain_converter: identity composition returns original value",
          "[mock][multi_value_converter]") {
    // Both stages are identities (string -> string).
    std::function<std::string(const std::string&)> id1 =
        [](const std::string& s) { return s; };
    std::function<std::string(const std::string&)> id2 =
        [](const std::string& s) { return s; };

    auto chain = chain_converter<std::string, std::string, std::string>(id1, id2);

    CHECK(chain("hello") == "hello");
    CHECK(chain("")      == "");
}

TEST_CASE("chain_converter: double -> string via two formatting stages",
          "[mock][multi_value_converter]") {
    // Stage 1: double -> int (truncate)
    std::function<int(const double&)> trunc =
        [](const double& d) { return static_cast<int>(d); };

    // Stage 2: int -> string (decimal)
    std::function<std::string(const int&)> to_str =
        [](const int& n) { return std::to_string(n); };

    auto chain = chain_converter<double, int, std::string>(trunc, to_str);

    CHECK(chain(3.9)  == "3");
    CHECK(chain(0.1)  == "0");
    CHECK(chain(-2.7) == "-2");
}
