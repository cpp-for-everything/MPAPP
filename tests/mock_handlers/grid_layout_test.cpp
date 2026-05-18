// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::grid_layout` (T-0011).

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/grid_layout.hpp>
#include <mpapp/handlers/mock/grid_layout_handler.hpp>

using namespace mpapp;

TEST_CASE("grid_layout mock handler records initial property values on bind",
          "[mock][grid_layout]") {
    grid_layout g;
    grid_layout_handler<platform::mock> h;

    h.map_row_count(g);
    h.map_column_count(g);
    h.map_row_spacing(g);
    h.map_column_spacing(g);

    REQUIRE(h.calls().size() == 4);
    CHECK(h.calls()[0].property_name == "row_count");
    CHECK(h.calls()[0].value_repr    == "1");
    CHECK(h.calls()[1].property_name == "column_count");
    CHECK(h.calls()[1].value_repr    == "1");
    CHECK(h.calls()[2].property_name == "row_spacing");
    CHECK(h.calls()[2].value_repr    == "0");
    CHECK(h.calls()[3].property_name == "column_spacing");
    CHECK(h.calls()[3].value_repr    == "0");
}

TEST_CASE("grid_layout mock handler fires once per real property change",
          "[mock][grid_layout]") {
    grid_layout g;
    grid_layout_handler<platform::mock> h;

    h.map_row_count(g);
    h.map_column_count(g);
    h.map_row_spacing(g);
    h.map_column_spacing(g);
    h.clear_calls();

    g.row_count      = 3;
    g.column_count   = 2;
    g.column_count   = 2;       // suppressed
    g.row_spacing    = 8.0;
    g.column_spacing = 4.0;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "row_count=3",
        "column_count=2",
        "row_spacing=8",
        "column_spacing=4",
    });
}
