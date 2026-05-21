// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::list_view`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/list_view_handler.hpp>
#include <mpapp/list_view.hpp>

using namespace mpapp;

TEST_CASE("list_view mock records initial values on bind",
          "[mock][list_view]") {
    list_view lv;
    list_view_handler<platform::mock> h;
    h.map_items_source(lv);
    h.map_selected_index(lv);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "items_source.count");
    CHECK(h.calls()[0].value_repr    == "0");
    CHECK(h.calls()[1].property_name == "selected_index");
    CHECK(h.calls()[1].value_repr    == "-1");
}

TEST_CASE("list_view records collection + selection changes",
          "[mock][list_view]") {
    list_view lv;
    list_view_handler<platform::mock> h;
    h.map_items_source(lv);
    h.map_selected_index(lv);
    h.clear_calls();

    lv.items_source   = std::vector<std::string>{"a", "b", "c"};
    lv.selected_index = 1;
    lv.selected_index = -1;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "items_source.count=3",
        "selected_index=1",
        "selected_index=-1",
    });
}
