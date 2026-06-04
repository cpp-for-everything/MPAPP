// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for converters_value_map.hpp (CTK batch 2).

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/binding/converters_value_map.hpp>
#include <mpapp/binding/binding.hpp>
#include <mpapp/observable.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// bool_to_object_converter
// ---------------------------------------------------------------------------

TEST_CASE("bool_to_object_converter maps true/false to T values",
          "[mock][converters_value_map]") {
    // Arrange
    bool_to_object_converter<std::string> c{ "yes", "no" };

    // Act + Assert
    CHECK(c.convert(true)  == "yes");
    CHECK(c.convert(false) == "no");
}

TEST_CASE("bool_to_object_converter convert_back: true when target == true_value",
          "[mock][converters_value_map]") {
    bool_to_object_converter<int> c{ 42, 0 };

    CHECK(c.convert_back(42) == true);
    CHECK(c.convert_back(0)  == false);
    CHECK(c.convert_back(7)  == false);   // not equal to true_value
}

TEST_CASE("bool_to_object_converter callable operator works",
          "[mock][converters_value_map]") {
    bool_to_object_converter<std::string> c{ "on", "off" };

    CHECK(c(true)  == "on");
    CHECK(c(false) == "off");
}

TEST_CASE("bool_to_object_converter default-constructed yields T{} for both",
          "[mock][converters_value_map]") {
    bool_to_object_converter<int> c;   // true_value_ == 0, false_value_ == 0

    CHECK(c.convert(true)  == 0);
    CHECK(c.convert(false) == 0);
}

TEST_CASE("bool_to_object_converter plugs into binding",
          "[mock][converters_value_map][binding]") {
    // Arrange
    Observable<bool>        flag{ false };
    Observable<std::string> label{ "" };
    bool_to_object_converter<std::string> c{ "active", "inactive" };

    // Act
    binding<bool, std::string> b{
        flag, label, binding_mode::one_way,
        [&c](const bool& v) { return c(v); }
    };

    // Assert initial
    CHECK(label.get() == "inactive");
    flag = true;
    CHECK(label.get() == "active");
}

// ---------------------------------------------------------------------------
// enum_to_bool_converter
// ---------------------------------------------------------------------------

enum class color : std::uint8_t { red, green, blue };

TEST_CASE("enum_to_bool_converter returns true only for matching value",
          "[mock][converters_value_map]") {
    enum_to_bool_converter<color> c{ color::green };

    CHECK(c.convert(color::green) == true);
    CHECK(c.convert(color::red)   == false);
    CHECK(c.convert(color::blue)  == false);
}

TEST_CASE("enum_to_bool_converter convert_back returns match on true",
          "[mock][converters_value_map]") {
    enum_to_bool_converter<color> c{ color::blue };

    CHECK(c.convert_back(true)  == color::blue);
    CHECK(c.convert_back(false) == color{});   // E{} == color::red (first enumerator)
}

TEST_CASE("enum_to_bool_converter callable operator works",
          "[mock][converters_value_map]") {
    enum_to_bool_converter<color> c{ color::red };

    CHECK(c(color::red)   == true);
    CHECK(c(color::green) == false);
}

TEST_CASE("enum_to_bool_converter default-constructed matches E{}",
          "[mock][converters_value_map]") {
    enum_to_bool_converter<color> c;   // match_ == color::red (E{})

    CHECK(c.convert(color::red)   == true);
    CHECK(c.convert(color::green) == false);
}

// ---------------------------------------------------------------------------
// int_to_bool_converter
// ---------------------------------------------------------------------------

TEST_CASE("int_to_bool_converter: 0 -> false, non-zero -> true",
          "[mock][converters_value_map]") {
    int_to_bool_converter c;

    CHECK(c.convert(0)    == false);
    CHECK(c.convert(1)    == true);
    CHECK(c.convert(-1)   == true);
    CHECK(c.convert(1000) == true);
}

TEST_CASE("int_to_bool_converter convert_back: true->1, false->0",
          "[mock][converters_value_map]") {
    int_to_bool_converter c;

    CHECK(c.convert_back(true)  == 1);
    CHECK(c.convert_back(false) == 0);
}

TEST_CASE("int_to_bool_converter callable operator mirrors convert",
          "[mock][converters_value_map]") {
    int_to_bool_converter c;

    CHECK(c(0)  == false);
    CHECK(c(42) == true);
}

TEST_CASE("int_to_bool_converter plugs into binding",
          "[mock][converters_value_map][binding]") {
    Observable<int>  count{ 0 };
    Observable<bool> has_items{ false };
    int_to_bool_converter c;

    binding<int, bool> b{
        count, has_items, binding_mode::one_way,
        [&c](const int& v) { return c(v); }
    };

    CHECK(has_items.get() == false);
    count = 3;
    CHECK(has_items.get() == true);
    count = 0;
    CHECK(has_items.get() == false);
}

// ---------------------------------------------------------------------------
// index_to_array_item_converter
// ---------------------------------------------------------------------------

TEST_CASE("index_to_array_item_converter returns element at valid index",
          "[mock][converters_value_map]") {
    index_to_array_item_converter<std::string> c{
        std::vector<std::string>{ "alpha", "beta", "gamma" }
    };

    CHECK(c.convert(0) == std::optional<std::string>{ "alpha" });
    CHECK(c.convert(1) == std::optional<std::string>{ "beta"  });
    CHECK(c.convert(2) == std::optional<std::string>{ "gamma" });
}

TEST_CASE("index_to_array_item_converter returns nullopt for out-of-range",
          "[mock][converters_value_map]") {
    index_to_array_item_converter<std::string> c{
        std::vector<std::string>{ "only" }
    };

    CHECK(c.convert(-1) == std::nullopt);
    CHECK(c.convert(1)  == std::nullopt);
    CHECK(c.convert(99) == std::nullopt);
}

TEST_CASE("index_to_array_item_converter returns nullopt for empty vector",
          "[mock][converters_value_map]") {
    index_to_array_item_converter<int> c;

    CHECK(c.convert(0) == std::nullopt);
}

TEST_CASE("index_to_array_item_converter convert_back finds item index",
          "[mock][converters_value_map]") {
    index_to_array_item_converter<int> c{ std::vector<int>{ 10, 20, 30 } };

    CHECK(c.convert_back(std::optional<int>{ 10 }) == 0);
    CHECK(c.convert_back(std::optional<int>{ 20 }) == 1);
    CHECK(c.convert_back(std::optional<int>{ 30 }) == 2);
}

TEST_CASE("index_to_array_item_converter convert_back returns -1 when not found",
          "[mock][converters_value_map]") {
    index_to_array_item_converter<int> c{ std::vector<int>{ 10, 20 } };

    CHECK(c.convert_back(std::optional<int>{ 99 }) == -1);
    CHECK(c.convert_back(std::nullopt)             == -1);
}

TEST_CASE("index_to_array_item_converter callable operator works",
          "[mock][converters_value_map]") {
    index_to_array_item_converter<std::string> c{
        std::vector<std::string>{ "x", "y" }
    };

    CHECK(c(0) == std::optional<std::string>{ "x" });
    CHECK(c(5) == std::nullopt);
}

// ---------------------------------------------------------------------------
// item_to_index_converter
// ---------------------------------------------------------------------------

TEST_CASE("item_to_index_converter returns zero-based index of item",
          "[mock][converters_value_map]") {
    item_to_index_converter<std::string> c{
        std::vector<std::string>{ "apple", "banana", "cherry" }
    };

    CHECK(c.convert("apple")  == 0);
    CHECK(c.convert("banana") == 1);
    CHECK(c.convert("cherry") == 2);
}

TEST_CASE("item_to_index_converter returns -1 when item not in list",
          "[mock][converters_value_map]") {
    item_to_index_converter<std::string> c{
        std::vector<std::string>{ "a", "b" }
    };

    CHECK(c.convert("z")  == -1);
    CHECK(c.convert("")   == -1);
}

TEST_CASE("item_to_index_converter returns -1 for empty vector",
          "[mock][converters_value_map]") {
    item_to_index_converter<int> c;

    CHECK(c.convert(0) == -1);
}

TEST_CASE("item_to_index_converter convert_back returns item at index",
          "[mock][converters_value_map]") {
    item_to_index_converter<std::string> c{
        std::vector<std::string>{ "one", "two", "three" }
    };

    CHECK(c.convert_back(0) == "one");
    CHECK(c.convert_back(1) == "two");
    CHECK(c.convert_back(2) == "three");
}

TEST_CASE("item_to_index_converter convert_back returns T{} for out-of-range",
          "[mock][converters_value_map]") {
    item_to_index_converter<std::string> c{
        std::vector<std::string>{ "x" }
    };

    CHECK(c.convert_back(-1) == std::string{});
    CHECK(c.convert_back(1)  == std::string{});
    CHECK(c.convert_back(99) == std::string{});
}

TEST_CASE("item_to_index_converter callable operator works",
          "[mock][converters_value_map]") {
    item_to_index_converter<int> c{ std::vector<int>{ 5, 10, 15 } };

    CHECK(c(5)  == 0);
    CHECK(c(15) == 2);
    CHECK(c(99) == -1);
}

TEST_CASE("item_to_index_converter plugs into binding",
          "[mock][converters_value_map][binding]") {
    // Arrange: selected tab name -> tab index
    Observable<std::string> tab_name{ "home" };
    Observable<int>         tab_index{ -1 };

    item_to_index_converter<std::string> c{
        std::vector<std::string>{ "home", "search", "profile" }
    };

    // Act
    binding<std::string, int> b{
        tab_name, tab_index, binding_mode::one_way,
        [&c](const std::string& name) { return c(name); }
    };

    // Assert
    CHECK(tab_index.get() == 0);
    tab_name = std::string{ "profile" };
    CHECK(tab_index.get() == 2);
    tab_name = std::string{ "unknown" };
    CHECK(tab_index.get() == -1);
}
