// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::menu_flyout` (M-04b).

#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/menu_flyout_handler.hpp>
#include <mpapp/menu_flyout.hpp>
#include <mpapp/menu_flyout_item.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

TEST_CASE("menu_flyout mock handler records initial property values on bind",
          "[mock][menu_flyout]") {
    menu_flyout f;
    menu_flyout_handler<platform::mock> h;

    h.map_items(f);
    h.map_is_open(f);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "items.count");
    CHECK(h.calls()[0].value_repr    == "0");
    CHECK(h.calls()[1].property_name == "is_open");
    CHECK(h.calls()[1].value_repr    == "false");
}

TEST_CASE("menu_flyout mock handler tracks items + is_open changes",
          "[mock][menu_flyout]") {
    menu_flyout f;
    menu_flyout_handler<platform::mock> h;

    h.map_items(f);
    h.map_is_open(f);
    h.clear_calls();

    menu_flyout_item a, b;
    f.items     = std::vector<view*>{&a, &b};
    f.is_open   = true;
    f.is_open   = true;  // idempotent — no extra record
    f.items     = std::vector<view*>{&a};
    f.is_open   = false;

    REQUIRE(h.calls().size() == 4);
    CHECK(h.calls()[0].property_name == "items.count");
    CHECK(h.calls()[0].value_repr    == "2");
    CHECK(h.calls()[1].property_name == "is_open");
    CHECK(h.calls()[1].value_repr    == "true");
    CHECK(h.calls()[2].property_name == "items.count");
    CHECK(h.calls()[2].value_repr    == "1");
    CHECK(h.calls()[3].property_name == "is_open");
    CHECK(h.calls()[3].value_repr    == "false");
}
