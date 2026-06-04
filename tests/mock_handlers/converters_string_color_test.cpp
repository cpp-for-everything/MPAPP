// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for CTK converters batch 3 (string/color).

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/binding/converters_string_color.hpp>
#include <mpapp/color.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// color_to_hex
// ---------------------------------------------------------------------------

TEST_CASE("color_to_hex - opaque colors produce #RRGGBB", "[mock][converters_string_color]") {
    // Arrange
    const color red   = color::from_rgb8(255, 0, 0);
    const color green = color::from_rgb8(0, 255, 0);
    const color black = color::from_rgb8(0, 0, 0);
    const color white = color::from_rgb8(255, 255, 255);

    color_to_hex_converter c;

    // Act + Assert
    CHECK(c.convert(red)   == "#FF0000");
    CHECK(c.convert(green) == "#00FF00");
    CHECK(c.convert(black) == "#000000");
    CHECK(c.convert(white) == "#FFFFFF");
}

TEST_CASE("color_to_hex - with alpha produces #RRGGBBAA", "[mock][converters_string_color]") {
    // Arrange
    const color semi = color::from_rgb8(0, 128, 255, 128);
    const color full = color::from_rgb8(255, 255, 255, 255);
    const color none_a = color::from_rgb8(0, 0, 0, 0);

    color_to_hex_converter c{ true }; // include_alpha = true

    // Act + Assert
    CHECK(c.convert(semi)   == "#0080FF80");
    CHECK(c.convert(full)   == "#FFFFFFFF");
    CHECK(c.convert(none_a) == "#00000000");
}

TEST_CASE("color_to_hex - free-function helper", "[mock][converters_string_color]") {
    // Arrange
    const color blue = color::from_rgb8(0, 0, 255);
    auto fn_no_alpha = color_to_hex();
    auto fn_alpha    = color_to_hex(true);

    // Act + Assert
    CHECK(fn_no_alpha(blue) == "#0000FF");
    CHECK(fn_alpha(blue)    == "#0000FFFF");
}

TEST_CASE("color_to_hex_converter - convert_back round-trip RGB", "[mock][converters_string_color]") {
    // Arrange
    color_to_hex_converter c{ false };
    const color original = color::from_rgb8(18, 52, 86);

    // Act
    const std::string hex = c.convert(original);
    const color back      = c.convert_back(hex);

    // Assert
    CHECK(hex == "#123456");
    CHECK(back.r == original.r);
    CHECK(back.g == original.g);
    CHECK(back.b == original.b);
    // alpha defaults to 1.0 when not in the hex string
    CHECK(back.a == 1.0);
}

TEST_CASE("color_to_hex_converter - convert_back round-trip RGBA", "[mock][converters_string_color]") {
    // Arrange
    color_to_hex_converter c{ true };
    const color original = color::from_rgb8(18, 52, 86, 170);

    // Act
    const std::string hex = c.convert(original);
    const color back      = c.convert_back(hex);

    // Assert
    CHECK(hex == "#123456AA");
    // Verify channels individually (floating-point round-trip via byte)
    CHECK(detail::to_byte(back.r) == 18);
    CHECK(detail::to_byte(back.g) == 52);
    CHECK(detail::to_byte(back.b) == 86);
    CHECK(detail::to_byte(back.a) == 170);
}

TEST_CASE("color_to_hex_converter - convert_back handles bad input gracefully",
          "[mock][converters_string_color]") {
    color_to_hex_converter c;
    const color result = c.convert_back("not-a-color");
    CHECK(result.r == 0.0);
    CHECK(result.g == 0.0);
    CHECK(result.b == 0.0);
}

TEST_CASE("color_to_hex - channel clamping at boundaries", "[mock][converters_string_color]") {
    // Values outside [0,1] should clamp to 0/255.
    color over  = { 1.5, -0.1, 0.5, 2.0 };
    color_to_hex_converter c{ true };
    CHECK(c.convert(over) == "#FF00" + std::string("80") + "FF");
}

// ---------------------------------------------------------------------------
// color_to_rgba_string
// ---------------------------------------------------------------------------

TEST_CASE("color_to_rgba_string - basic conversion", "[mock][converters_string_color]") {
    // Arrange
    const color red  = color::from_rgb8(255, 0, 0, 255);
    const color semi = color::from_rgb8(0, 128, 255, 64);

    color_to_rgba_string_converter c;

    // Act + Assert
    CHECK(c.convert(red)  == "rgba(255,0,0,255)");
    CHECK(c.convert(semi) == "rgba(0,128,255,64)");
}

TEST_CASE("color_to_rgba_string - free-function helper", "[mock][converters_string_color]") {
    // Arrange
    const color white = color::from_rgb8(255, 255, 255, 255);
    auto fn = color_to_rgba_string();

    // Act + Assert
    CHECK(fn(white) == "rgba(255,255,255,255)");
}

TEST_CASE("color_to_rgba_string_converter - convert_back returns default",
          "[mock][converters_string_color]") {
    color_to_rgba_string_converter c;
    const color result = c.convert_back("rgba(1,2,3,4)");
    CHECK(result.r == 0.0);
    CHECK(result.g == 0.0);
    CHECK(result.b == 0.0);
    CHECK(result.a == 1.0);
}

// ---------------------------------------------------------------------------
// text_case
// ---------------------------------------------------------------------------

TEST_CASE("text_case - none leaves string unchanged", "[mock][converters_string_color]") {
    text_case_converter c{ text_case::none };
    CHECK(c.convert("Hello World") == "Hello World");
    CHECK(c.convert("")            == "");
}

TEST_CASE("text_case - upper converts to uppercase", "[mock][converters_string_color]") {
    text_case_converter c{ text_case::upper };
    CHECK(c.convert("hello world") == "HELLO WORLD");
    CHECK(c.convert("MiXeD")       == "MIXED");
    CHECK(c.convert("")            == "");
}

TEST_CASE("text_case - lower converts to lowercase", "[mock][converters_string_color]") {
    text_case_converter c{ text_case::lower };
    CHECK(c.convert("HELLO WORLD") == "hello world");
    CHECK(c.convert("MiXeD")       == "mixed");
    CHECK(c.convert("")            == "");
}

TEST_CASE("text_case - title capitalises each word", "[mock][converters_string_color]") {
    text_case_converter c{ text_case::title };
    CHECK(c.convert("hello world")    == "Hello World");
    CHECK(c.convert("HELLO WORLD")    == "Hello World");
    CHECK(c.convert("the quick brown fox") == "The Quick Brown Fox");
    CHECK(c.convert("one")            == "One");
    CHECK(c.convert("")               == "");
}

TEST_CASE("text_case - title-case handles leading/trailing spaces",
          "[mock][converters_string_color]") {
    text_case_converter c{ text_case::title };
    CHECK(c.convert("  hi there  ") == "  Hi There  ");
}

TEST_CASE("text_case - convert_back returns unchanged string",
          "[mock][converters_string_color]") {
    text_case_converter c{ text_case::upper };
    CHECK(c.convert_back("ANYTHING") == "ANYTHING");
}

TEST_CASE("text_case - free-function make_text_case", "[mock][converters_string_color]") {
    auto upper_fn = make_text_case(text_case::upper);
    auto lower_fn = make_text_case(text_case::lower);
    auto title_fn = make_text_case(text_case::title);

    CHECK(upper_fn("hello") == "HELLO");
    CHECK(lower_fn("HELLO") == "hello");
    CHECK(title_fn("hello world") == "Hello World");
}

// ---------------------------------------------------------------------------
// list_to_string
// ---------------------------------------------------------------------------

TEST_CASE("list_to_string - joins with comma by default", "[mock][converters_string_color]") {
    list_to_string_converter c;
    CHECK(c.convert({"a", "b", "c"}) == "a,b,c");
    CHECK(c.separator() == ",");
}

TEST_CASE("list_to_string - joins with custom separator", "[mock][converters_string_color]") {
    list_to_string_converter c{ " | " };
    CHECK(c.convert({"foo", "bar", "baz"}) == "foo | bar | baz");
}

TEST_CASE("list_to_string - empty list produces empty string", "[mock][converters_string_color]") {
    list_to_string_converter c;
    CHECK(c.convert({}) == "");
}

TEST_CASE("list_to_string - single element produces no separator", "[mock][converters_string_color]") {
    list_to_string_converter c{ "," };
    CHECK(c.convert({"only"}) == "only");
}

TEST_CASE("list_to_string - free-function helper", "[mock][converters_string_color]") {
    auto fn = list_to_string("-");
    CHECK(fn({"x", "y", "z"}) == "x-y-z");
}

TEST_CASE("list_to_string - round-trip through convert_back", "[mock][converters_string_color]") {
    // Arrange
    list_to_string_converter c{ "," };
    const std::vector<std::string> original = { "apple", "banana", "cherry" };

    // Act
    const std::string joined = c.convert(original);
    const std::vector<std::string> back = c.convert_back(joined);

    // Assert
    CHECK(joined == "apple,banana,cherry");
    CHECK(back == original);
}

// ---------------------------------------------------------------------------
// string_to_list
// ---------------------------------------------------------------------------

TEST_CASE("string_to_list - splits on comma by default", "[mock][converters_string_color]") {
    string_to_list_converter c;
    CHECK(c.convert("a,b,c") == std::vector<std::string>{"a", "b", "c"});
    CHECK(c.separator() == ",");
}

TEST_CASE("string_to_list - splits on multi-char separator", "[mock][converters_string_color]") {
    string_to_list_converter c{ " | " };
    CHECK(c.convert("foo | bar | baz") ==
          std::vector<std::string>{"foo", "bar", "baz"});
}

TEST_CASE("string_to_list - empty string yields one empty element", "[mock][converters_string_color]") {
    string_to_list_converter c{ "," };
    const auto result = c.convert("");
    REQUIRE(result.size() == 1);
    CHECK(result[0] == "");
}

TEST_CASE("string_to_list - no separator found yields whole string as single element",
          "[mock][converters_string_color]") {
    string_to_list_converter c{ "," };
    CHECK(c.convert("hello") == std::vector<std::string>{"hello"});
}

TEST_CASE("string_to_list - trailing separator produces trailing empty element",
          "[mock][converters_string_color]") {
    string_to_list_converter c{ "," };
    const auto result = c.convert("a,b,");
    REQUIRE(result.size() == 3);
    CHECK(result[2] == "");
}

TEST_CASE("string_to_list - free-function helper", "[mock][converters_string_color]") {
    auto fn = string_to_list(";");
    const auto result = fn("x;y;z");
    CHECK(result == std::vector<std::string>{"x", "y", "z"});
}

TEST_CASE("string_to_list - round-trip through convert_back", "[mock][converters_string_color]") {
    // Arrange
    string_to_list_converter c{ "," };
    const std::string original = "red,green,blue";

    // Act
    const auto list    = c.convert(original);
    const std::string back = c.convert_back(list);

    // Assert
    CHECK(list == std::vector<std::string>{"red", "green", "blue"});
    CHECK(back == original);
}

// ---------------------------------------------------------------------------
// Cross-converter: list -> string -> list round-trip (using both converters)
// ---------------------------------------------------------------------------

TEST_CASE("list/string cross-converter round-trip", "[mock][converters_string_color]") {
    const std::vector<std::string> words = { "one", "two", "three" };

    auto join_fn  = list_to_string("|");
    auto split_fn = string_to_list("|");

    const std::string joined = join_fn(words);
    CHECK(joined == "one|two|three");

    const auto split_back = split_fn(joined);
    CHECK(split_back == words);
}
