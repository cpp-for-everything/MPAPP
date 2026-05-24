// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_title_bar`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/title_bar_handler.hpp>
#include <mpapp/title_bar.hpp>

using namespace mpapp;

TEST_CASE("title_bar mock records initial values on bind",
          "[mock][title_bar]") {
    internal::basic_title_bar t;
    title_bar_handler<platform::mock> h;

    h.map_title(t);
    h.map_subtitle(t);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "title");
    CHECK(h.calls()[0].value_repr    == "");
    CHECK(h.calls()[1].property_name == "subtitle");
    CHECK(h.calls()[1].value_repr    == "");
}

TEST_CASE("title_bar records title and subtitle changes",
          "[mock][title_bar]") {
    internal::basic_title_bar t;
    title_bar_handler<platform::mock> h;

    h.map_title(t);
    h.map_subtitle(t);
    h.clear_calls();

    t.title = "Notes";
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "title");
    CHECK(h.calls()[0].value_repr    == "Notes");

    t.title = "Notes";          // idempotent — no re-emit on equal set
    REQUIRE(h.calls().size() == 1);

    t.subtitle = "Untitled";
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].property_name == "subtitle");
    CHECK(h.calls()[1].value_repr    == "Untitled");
}
