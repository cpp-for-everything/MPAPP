// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_menu_bar`.

#include <catch2/catch_test_macros.hpp>

#include <vector>

#include <mpapp/handlers/mock/menu_bar_handler.hpp>
#include <mpapp/menu_bar.hpp>
#include <mpapp/menu_bar_item.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

TEST_CASE("menu_bar mock records initial items count on bind",
          "[mock][menu_bar]") {
    internal::basic_menu_bar mb;
    menu_bar_handler<platform::mock> h;

    h.map_items(mb);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "items.count");
    CHECK(h.calls()[0].value_repr    == "0");
}

TEST_CASE("menu_bar records items.count when collection changes",
          "[mock][menu_bar]") {
    internal::basic_menu_bar mb;
    menu_bar_handler<platform::mock> h;

    h.map_items(mb);
    h.clear_calls();

    internal::basic_menu_bar_item file;
    internal::basic_menu_bar_item edit;
    internal::basic_menu_bar_item view_;

    mb.items = std::vector<view*>{ &file, &edit, &view_ };
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "items.count");
    CHECK(h.calls()[0].value_repr    == "3");

    // Idempotent — equal vector should not re-record.
    mb.items = std::vector<view*>{ &file, &edit, &view_ };
    REQUIRE(h.calls().size() == 1);

    mb.items = std::vector<view*>{};
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "0");
}
