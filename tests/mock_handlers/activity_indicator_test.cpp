// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_activity_indicator`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_activity_indicator.hpp>
#include <mpapp/handlers/mock/activity_indicator_handler.hpp>

using namespace mpapp;

TEST_CASE("activity_indicator mock records initial values on bind",
          "[mock][activity_indicator]") {
    internal::basic_activity_indicator a;
    internal::activity_indicator_handler<platform::mock> h;

    h.map_is_running(a);
    h.map_color(a);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "is_running");
    CHECK(h.calls()[0].value_repr    == "false");
    CHECK(h.calls()[1].property_name == "color");
}

TEST_CASE("activity_indicator records start/stop transitions",
          "[mock][activity_indicator]") {
    internal::basic_activity_indicator a;
    internal::activity_indicator_handler<platform::mock> h;

    h.map_is_running(a);
    h.clear_calls();

    a.is_running = true;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "true");

    a.is_running = true;                // idempotent
    REQUIRE(h.calls().size() == 1);

    a.is_running = false;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "false");
}
