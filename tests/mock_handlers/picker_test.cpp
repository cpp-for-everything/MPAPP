// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_picker`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/picker_handler.hpp>
#include <mpapp/internal/basic_picker.hpp>

using namespace mpapp;

TEST_CASE("picker mock records initial values on bind",
          "[mock][picker]") {
    internal::basic_picker p;
    internal::picker_handler<platform::mock> h;

    h.map_items(p);
    h.map_selected_index(p);
    h.map_title(p);

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "items.count");
    CHECK(h.calls()[0].value_repr    == "0");
    CHECK(h.calls()[1].property_name == "selected_index");
    CHECK(h.calls()[1].value_repr    == "-1");
    CHECK(h.calls()[2].property_name == "title");
}

TEST_CASE("picker records items.count when collection changes",
          "[mock][picker]") {
    internal::basic_picker p;
    internal::picker_handler<platform::mock> h;

    h.map_items(p);
    h.clear_calls();

    p.items = std::vector<std::string>{"Red", "Green", "Blue"};
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "items.count");
    CHECK(h.calls()[0].value_repr    == "3");
}

TEST_CASE("picker records selection changes",
          "[mock][picker]") {
    internal::basic_picker p;
    internal::picker_handler<platform::mock> h;

    h.map_selected_index(p);
    h.clear_calls();

    p.selected_index = 2;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "2");

    p.selected_index = 2;            // idempotent
    REQUIRE(h.calls().size() == 1);

    p.selected_index = -1;
    REQUIRE(h.calls().size() == 2);
}
