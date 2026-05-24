// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_tabbed_view`.

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include <mpapp/handlers/mock/tabbed_view_handler.hpp>
#include <mpapp/tabbed_view.hpp>

using namespace mpapp;

TEST_CASE("tabbed_view mock records initial values on bind",
          "[mock][tabbed_view]") {
    internal::basic_tabbed_view tv;
    tabbed_view_handler<platform::mock> h;

    h.map_tab_titles(tv);
    h.map_selected_index(tv);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "tab_titles.count");
    CHECK(h.calls()[0].value_repr    == "0");
    CHECK(h.calls()[1].property_name == "selected_index");
    CHECK(h.calls()[1].value_repr    == "-1");
}

TEST_CASE("tabbed_view records tab_titles.count when collection changes",
          "[mock][tabbed_view]") {
    internal::basic_tabbed_view tv;
    tabbed_view_handler<platform::mock> h;

    h.map_tab_titles(tv);
    h.clear_calls();

    tv.tab_titles = std::vector<std::string>{"Inbox", "Drafts", "Sent"};
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "tab_titles.count");
    CHECK(h.calls()[0].value_repr    == "3");

    // Shrink the collection — the count records the new size.
    tv.tab_titles = std::vector<std::string>{"Inbox"};
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr    == "1");
}

TEST_CASE("tabbed_view records selection changes",
          "[mock][tabbed_view]") {
    internal::basic_tabbed_view tv;
    tabbed_view_handler<platform::mock> h;

    h.map_selected_index(tv);
    h.clear_calls();

    tv.selected_index = 2;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "selected_index");
    CHECK(h.calls()[0].value_repr    == "2");

    tv.selected_index = 2;            // idempotent — no extra row
    REQUIRE(h.calls().size() == 1);

    tv.selected_index = -1;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr    == "-1");
}
