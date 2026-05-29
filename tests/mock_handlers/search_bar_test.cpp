// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_search_bar`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/search_bar_handler.hpp>
#include <mpapp/internal/basic_search_bar.hpp>

using namespace mpapp;

TEST_CASE("search_bar mock records initial values on bind",
          "[mock][search_bar]") {
    internal::basic_search_bar s;
    internal::search_bar_handler<platform::mock> h;

    h.map_text(s);
    h.map_placeholder(s);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "text");
    CHECK(h.calls()[0].value_repr    == "");
    CHECK(h.calls()[1].property_name == "placeholder");
}

TEST_CASE("search_bar records text changes",
          "[mock][search_bar]") {
    internal::basic_search_bar s;
    internal::search_bar_handler<platform::mock> h;

    h.map_text(s);
    h.clear_calls();

    s.text = "hello";
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "hello");

    s.text = "hello";           // idempotent
    REQUIRE(h.calls().size() == 1);

    s.text = "world";
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "world");
}
