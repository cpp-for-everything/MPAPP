// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_time_picker`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/time_picker_handler.hpp>
#include <mpapp/internal/basic_time_picker.hpp>

using namespace mpapp;

TEST_CASE("time_picker mock records initial values on bind",
          "[mock][time_picker]") {
    internal::basic_time_picker p;
    internal::time_picker_handler<platform::mock> h;

    h.map_time(p);
    h.map_format(p);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "time");
    CHECK(h.calls()[0].value_repr    == "00:00");
}

TEST_CASE("time_picker records time changes",
          "[mock][time_picker]") {
    internal::basic_time_picker p;
    internal::time_picker_handler<platform::mock> h;

    h.map_time(p);
    h.clear_calls();

    p.time = time_value{14, 30};
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "14:30");
}
