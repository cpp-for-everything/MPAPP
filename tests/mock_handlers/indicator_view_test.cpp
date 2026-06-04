// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_indicator_view`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/indicator_view_handler.hpp>
#include <mpapp/internal/basic_indicator_view.hpp>

using namespace mpapp;

TEST_CASE("indicator_view mock records initial values on bind",
          "[mock][indicator_view]") {
    internal::basic_indicator_view iv;
    internal::indicator_view_handler<platform::mock> h;

    h.map_count(iv);
    h.map_position(iv);
    h.map_indicator_color(iv);
    h.map_selected_indicator_color(iv);

    REQUIRE(h.calls().size() == 4);
    CHECK(h.calls()[0].property_name == "count");
    CHECK(h.calls()[0].value_repr    == "0");
    CHECK(h.calls()[1].property_name == "position");
    CHECK(h.calls()[1].value_repr    == "0");
    CHECK(h.calls()[2].property_name == "indicator_color");
    CHECK(h.calls()[3].property_name == "selected_indicator_color");
}

TEST_CASE("indicator_view records count changes",
          "[mock][indicator_view]") {
    internal::basic_indicator_view iv;
    internal::indicator_view_handler<platform::mock> h;

    h.map_count(iv);
    h.clear_calls();

    iv.count = 3;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "count");
    CHECK(h.calls()[0].value_repr    == "3");

    iv.count = 3;                  // idempotent - no extra record
    REQUIRE(h.calls().size() == 1);

    iv.count = 5;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "5");
}
