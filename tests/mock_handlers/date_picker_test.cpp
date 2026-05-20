// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::date_picker`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/date_picker.hpp>
#include <mpapp/handlers/mock/date_picker_handler.hpp>

using namespace mpapp;

TEST_CASE("date_picker mock records initial values on bind",
          "[mock][date_picker]") {
    date_picker p;
    date_picker_handler<platform::mock> h;

    h.map_date(p);
    h.map_format(p);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "date");
    CHECK(h.calls()[0].value_repr    == "1970-01-01");
    CHECK(h.calls()[1].property_name == "format");
}

TEST_CASE("date_picker records date changes",
          "[mock][date_picker]") {
    date_picker p;
    date_picker_handler<platform::mock> h;

    h.map_date(p);
    h.clear_calls();

    p.date = date_value{2026, 5, 20};
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "2026-05-20");

    p.date = date_value{2026, 5, 20};   // idempotent
    REQUIRE(h.calls().size() == 1);
}
