// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for converters_ctk2.hpp (CTK batch 3).

#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/binding/converters_ctk2.hpp>
#include <mpapp/binding/binding.hpp>
#include <mpapp/observable.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// is_equal<T>
// ---------------------------------------------------------------------------

TEST_CASE("is_equal returns true only when source matches target",
          "[mock][converters_ctk2]") {
    // Arrange
    is_equal<int> c{ 42 };

    // Act + Assert
    CHECK(c.convert(42)  == true);
    CHECK(c.convert(0)   == false);
    CHECK(c.convert(-1)  == false);
}

TEST_CASE("is_equal convert_back returns the stored sentinel",
          "[mock][converters_ctk2]") {
    is_equal<std::string> c{ "hello" };

    CHECK(c.convert_back(true)  == "hello");
    CHECK(c.convert_back(false) == "hello");
}

TEST_CASE("is_equal callable operator mirrors convert",
          "[mock][converters_ctk2]") {
    is_equal<int> c{ 7 };

    CHECK(c(7)  == true);
    CHECK(c(8)  == false);
}

TEST_CASE("is_equal default-constructed compares against T{}",
          "[mock][converters_ctk2]") {
    is_equal<int> c;   // target_ == 0

    CHECK(c.convert(0)  == true);
    CHECK(c.convert(1)  == false);
}

TEST_CASE("is_equal plugs into binding",
          "[mock][converters_ctk2][binding]") {
    // Arrange
    Observable<int>  count{ 0 };
    Observable<bool> is_zero{ false };
    is_equal<int> c{ 0 };

    binding<int, bool> b{
        count, is_zero, binding_mode::one_way,
        [&c](const int& v) { return c(v); }
    };

    // Assert initial
    CHECK(is_zero.get() == true);
    count = 5;
    CHECK(is_zero.get() == false);
    count = 0;
    CHECK(is_zero.get() == true);
}

// ---------------------------------------------------------------------------
// is_not_equal<T>
// ---------------------------------------------------------------------------

TEST_CASE("is_not_equal returns true when source differs from target",
          "[mock][converters_ctk2]") {
    is_not_equal<int> c{ 0 };

    CHECK(c.convert(0)   == false);
    CHECK(c.convert(1)   == true);
    CHECK(c.convert(-99) == true);
}

TEST_CASE("is_not_equal convert_back returns the stored sentinel",
          "[mock][converters_ctk2]") {
    is_not_equal<std::string> c{ "sentinel" };

    CHECK(c.convert_back(true)  == "sentinel");
    CHECK(c.convert_back(false) == "sentinel");
}

TEST_CASE("is_not_equal callable operator mirrors convert",
          "[mock][converters_ctk2]") {
    is_not_equal<int> c{ 10 };

    CHECK(c(10) == false);
    CHECK(c(11) == true);
}

TEST_CASE("is_not_equal default-constructed compares against T{}",
          "[mock][converters_ctk2]") {
    is_not_equal<int> c;   // target_ == 0

    CHECK(c.convert(0) == false);
    CHECK(c.convert(1) == true);
}

// ---------------------------------------------------------------------------
// is_in_range<T>
// ---------------------------------------------------------------------------

TEST_CASE("is_in_range returns true when value is within closed interval",
          "[mock][converters_ctk2]") {
    // Arrange
    is_in_range<int> c{ 1, 10 };

    // Act + Assert (boundary conditions)
    CHECK(c.convert(1)  == true);
    CHECK(c.convert(5)  == true);
    CHECK(c.convert(10) == true);
    CHECK(c.convert(0)  == false);
    CHECK(c.convert(11) == false);
}

TEST_CASE("is_in_range convert_back returns min",
          "[mock][converters_ctk2]") {
    is_in_range<int> c{ 5, 15 };

    CHECK(c.convert_back(true)  == 5);
    CHECK(c.convert_back(false) == 5);
}

TEST_CASE("is_in_range callable operator mirrors convert",
          "[mock][converters_ctk2]") {
    is_in_range<double> c{ 0.0, 1.0 };

    CHECK(c(0.0)  == true);
    CHECK(c(0.5)  == true);
    CHECK(c(1.0)  == true);
    CHECK(c(-0.1) == false);
    CHECK(c(1.1)  == false);
}

TEST_CASE("is_in_range default-constructed: min == max == T{}",
          "[mock][converters_ctk2]") {
    is_in_range<int> c;   // min_ == 0, max_ == 0

    CHECK(c.convert(0)  == true);
    CHECK(c.convert(1)  == false);
    CHECK(c.convert(-1) == false);
}

// ---------------------------------------------------------------------------
// clamp<T>
// ---------------------------------------------------------------------------

TEST_CASE("clamp constrains value to closed interval",
          "[mock][converters_ctk2]") {
    // Arrange
    clamp<int> c{ 0, 100 };

    // Act + Assert
    CHECK(c.convert(-10) == 0);
    CHECK(c.convert(0)   == 0);
    CHECK(c.convert(50)  == 50);
    CHECK(c.convert(100) == 100);
    CHECK(c.convert(200) == 100);
}

TEST_CASE("clamp convert_back also clamps",
          "[mock][converters_ctk2]") {
    clamp<int> c{ 0, 10 };

    CHECK(c.convert_back(-1)  == 0);
    CHECK(c.convert_back(5)   == 5);
    CHECK(c.convert_back(100) == 10);
}

TEST_CASE("clamp callable operator mirrors convert",
          "[mock][converters_ctk2]") {
    clamp<double> c{ -1.0, 1.0 };

    CHECK(c(-2.0) == -1.0);
    CHECK(c(0.0)  == 0.0);
    CHECK(c(2.0)  == 1.0);
}

TEST_CASE("clamp plugs into binding",
          "[mock][converters_ctk2][binding]") {
    // Arrange: raw slider value [0,255] clamped to [0,100] for a percentage
    Observable<int> raw{ 128 };
    Observable<int> pct{ 0 };
    clamp<int>      c{ 0, 100 };

    binding<int, int> b{
        raw, pct, binding_mode::one_way,
        [&c](const int& v) { return c(v); }
    };

    CHECK(pct.get() == 100);
    raw = 50;
    CHECK(pct.get() == 50);
    raw = -5;
    CHECK(pct.get() == 0);
}

// ---------------------------------------------------------------------------
// bool_negation
// ---------------------------------------------------------------------------

TEST_CASE("bool_negation inverts its input",
          "[mock][converters_ctk2]") {
    bool_negation c;

    CHECK(c.convert(true)  == false);
    CHECK(c.convert(false) == true);
}

TEST_CASE("bool_negation convert_back also inverts",
          "[mock][converters_ctk2]") {
    bool_negation c;

    CHECK(c.convert_back(true)  == false);
    CHECK(c.convert_back(false) == true);
}

TEST_CASE("bool_negation callable operator mirrors convert",
          "[mock][converters_ctk2]") {
    bool_negation c;

    CHECK(c(true)  == false);
    CHECK(c(false) == true);
}

TEST_CASE("bool_negation plugs into binding",
          "[mock][converters_ctk2][binding]") {
    Observable<bool> loading{ true };
    Observable<bool> content_visible{ false };
    bool_negation    c;

    binding<bool, bool> b{
        loading, content_visible, binding_mode::one_way,
        [&c](const bool& v) { return c(v); }
    };

    CHECK(content_visible.get() == false);
    loading = false;
    CHECK(content_visible.get() == true);
}

// ---------------------------------------------------------------------------
// double_to_int
// ---------------------------------------------------------------------------

TEST_CASE("double_to_int rounds to nearest integer",
          "[mock][converters_ctk2]") {
    double_to_int c;

    CHECK(c.convert(0.0)   == 0);
    CHECK(c.convert(1.4)   == 1);
    CHECK(c.convert(1.5)   == 2);
    CHECK(c.convert(1.6)   == 2);
    CHECK(c.convert(-1.5)  == -2);   // std::lround: round away from zero
    CHECK(c.convert(-1.4)  == -1);
}

TEST_CASE("double_to_int convert_back widens int to double",
          "[mock][converters_ctk2]") {
    double_to_int c;

    CHECK(c.convert_back(0)   == 0.0);
    CHECK(c.convert_back(7)   == 7.0);
    CHECK(c.convert_back(-3)  == -3.0);
}

TEST_CASE("double_to_int callable operator mirrors convert",
          "[mock][converters_ctk2]") {
    double_to_int c;

    CHECK(c(2.9)  == 3);
    CHECK(c(2.49) == 2);
}

// ---------------------------------------------------------------------------
// int_to_double
// ---------------------------------------------------------------------------

TEST_CASE("int_to_double widens int to double exactly",
          "[mock][converters_ctk2]") {
    int_to_double c;

    CHECK(c.convert(0)    == 0.0);
    CHECK(c.convert(42)   == 42.0);
    CHECK(c.convert(-100) == -100.0);
}

TEST_CASE("int_to_double convert_back rounds double back to int",
          "[mock][converters_ctk2]") {
    int_to_double c;

    CHECK(c.convert_back(0.0)  == 0);
    CHECK(c.convert_back(3.5)  == 4);
    CHECK(c.convert_back(-2.5) == -3);   // std::lround: round away from zero
}

TEST_CASE("int_to_double callable operator mirrors convert",
          "[mock][converters_ctk2]") {
    int_to_double c;

    CHECK(c(5)  == 5.0);
    CHECK(c(-1) == -1.0);
}

TEST_CASE("int_to_double plugs into binding",
          "[mock][converters_ctk2][binding]") {
    Observable<int>    steps{ 3 };
    Observable<double> ratio{ 0.0 };
    int_to_double      c;

    binding<int, double> b{
        steps, ratio, binding_mode::one_way,
        [&c](const int& v) { return c(v); }
    };

    CHECK(ratio.get() == 3.0);
    steps = -7;
    CHECK(ratio.get() == -7.0);
}

// ---------------------------------------------------------------------------
// string_to_upper
// ---------------------------------------------------------------------------

TEST_CASE("string_to_upper converts ASCII letters to upper-case",
          "[mock][converters_ctk2]") {
    string_to_upper c;

    CHECK(c.convert("hello")       == "HELLO");
    CHECK(c.convert("Hello World") == "HELLO WORLD");
    CHECK(c.convert("abc123")      == "ABC123");
    CHECK(c.convert("")            == "");
}

TEST_CASE("string_to_upper leaves non-alpha characters unchanged",
          "[mock][converters_ctk2]") {
    string_to_upper c;

    CHECK(c.convert("123!@#") == "123!@#");
    CHECK(c.convert("  ")     == "  ");
}

TEST_CASE("string_to_upper convert_back applies lower-case",
          "[mock][converters_ctk2]") {
    string_to_upper c;

    CHECK(c.convert_back("HELLO") == "hello");
    CHECK(c.convert_back("ABC")   == "abc");
}

TEST_CASE("string_to_upper callable operator mirrors convert",
          "[mock][converters_ctk2]") {
    string_to_upper c;

    CHECK(c(std::string{ "world" }) == "WORLD");
}

TEST_CASE("string_to_upper plugs into binding",
          "[mock][converters_ctk2][binding]") {
    Observable<std::string> input{ std::string{"hello"} };
    Observable<std::string> display{ std::string{""} };
    string_to_upper         c;

    binding<std::string, std::string> b{
        input, display, binding_mode::one_way,
        [&c](const std::string& v) { return c(v); }
    };

    CHECK(display.get() == "HELLO");
    input = std::string{ "world" };
    CHECK(display.get() == "WORLD");
}

// ---------------------------------------------------------------------------
// string_to_lower
// ---------------------------------------------------------------------------

TEST_CASE("string_to_lower converts ASCII letters to lower-case",
          "[mock][converters_ctk2]") {
    string_to_lower c;

    CHECK(c.convert("HELLO")       == "hello");
    CHECK(c.convert("Hello World") == "hello world");
    CHECK(c.convert("ABC123")      == "abc123");
    CHECK(c.convert("")            == "");
}

TEST_CASE("string_to_lower leaves non-alpha characters unchanged",
          "[mock][converters_ctk2]") {
    string_to_lower c;

    CHECK(c.convert("123!@#") == "123!@#");
    CHECK(c.convert("  ")     == "  ");
}

TEST_CASE("string_to_lower convert_back applies upper-case",
          "[mock][converters_ctk2]") {
    string_to_lower c;

    CHECK(c.convert_back("hello") == "HELLO");
    CHECK(c.convert_back("abc")   == "ABC");
}

TEST_CASE("string_to_lower callable operator mirrors convert",
          "[mock][converters_ctk2]") {
    string_to_lower c;

    CHECK(c(std::string{ "WORLD" }) == "world");
}

TEST_CASE("string_to_lower plugs into binding",
          "[mock][converters_ctk2][binding]") {
    Observable<std::string> input{ std::string{"SEARCH"} };
    Observable<std::string> normalized{ std::string{""} };
    string_to_lower         c;

    binding<std::string, std::string> b{
        input, normalized, binding_mode::one_way,
        [&c](const std::string& v) { return c(v); }
    };

    CHECK(normalized.get() == "search");
    input = std::string{ "QUERY" };
    CHECK(normalized.get() == "query");
}
