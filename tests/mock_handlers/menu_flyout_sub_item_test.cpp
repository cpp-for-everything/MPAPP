// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::internal::basic_menu_flyout_sub_item`
// (M-04b).

#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/menu_flyout_sub_item_handler.hpp>
#include <mpapp/internal/basic_menu_flyout_item.hpp>
#include <mpapp/internal/basic_menu_flyout_sub_item.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

TEST_CASE("menu_flyout_sub_item mock records initial property values on bind",
          "[mock][menu_flyout_sub_item]") {
    internal::basic_menu_flyout_sub_item s;
    internal::menu_flyout_sub_item_handler<platform::mock> h;

    h.map_text(s);
    h.map_items(s);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "text");
    CHECK(h.calls()[0].value_repr    == "");
    CHECK(h.calls()[1].property_name == "items.count");
    CHECK(h.calls()[1].value_repr    == "0");
}

TEST_CASE("menu_flyout_sub_item mock tracks text + items changes",
          "[mock][menu_flyout_sub_item]") {
    internal::basic_menu_flyout_sub_item s;
    internal::menu_flyout_sub_item_handler<platform::mock> h;

    h.map_text(s);
    h.map_items(s);
    h.clear_calls();

    internal::basic_menu_flyout_item a, b, c;
    s.text  = "Open with";
    s.items = std::vector<view*>{&a, &b, &c};
    s.items = std::vector<view*>{&a};
    s.text  = "Open with";  // idempotent — no row

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "text");
    CHECK(h.calls()[0].value_repr    == "Open with");
    CHECK(h.calls()[1].property_name == "items.count");
    CHECK(h.calls()[1].value_repr    == "3");
    CHECK(h.calls()[2].property_name == "items.count");
    CHECK(h.calls()[2].value_repr    == "1");
}
