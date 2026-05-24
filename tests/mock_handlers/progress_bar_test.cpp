// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_progress_bar`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/progress_bar_handler.hpp>
#include <mpapp/progress_bar.hpp>

using namespace mpapp;

TEST_CASE("progress_bar mock records initial values on bind",
          "[mock][progress_bar]") {
    internal::basic_progress_bar p;
    progress_bar_handler<platform::mock> h;

    h.map_progress(p);
    h.map_color(p);
    h.map_background_color(p);

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "progress");
    CHECK(h.calls()[0].value_repr    == "0");
    CHECK(h.calls()[1].property_name == "color");
    CHECK(h.calls()[2].property_name == "background_color");
}

TEST_CASE("progress_bar records progress changes",
          "[mock][progress_bar]") {
    internal::basic_progress_bar p;
    progress_bar_handler<platform::mock> h;

    h.map_progress(p);
    h.clear_calls();

    p.progress = 0.25;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "0.25");

    p.progress = 0.25;             // idempotent
    REQUIRE(h.calls().size() == 1);

    p.progress = 0.75;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "0.75");
}
