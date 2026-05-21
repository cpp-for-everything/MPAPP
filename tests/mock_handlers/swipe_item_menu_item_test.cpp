// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::swipe_item_menu_item`.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include <mpapp/handlers/mock/swipe_item_menu_item_handler.hpp>
#include <mpapp/swipe_item_menu_item.hpp>

using namespace mpapp;

TEST_CASE("swipe_item_menu_item mock records initial values on bind",
          "[mock][swipe_item_menu_item]") {
    swipe_item_menu_item m;
    swipe_item_menu_item_handler<platform::mock> h;

    h.map_text(m);
    h.map_icon_uri(m);
    h.map_invoked(m);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "text");
    CHECK(h.calls()[0].value_repr    == "");
    CHECK(h.calls()[1].property_name == "icon_uri");
    CHECK(h.calls()[1].value_repr    == "");
}

TEST_CASE("swipe_item_menu_item mock records text + icon_uri changes",
          "[mock][swipe_item_menu_item]") {
    swipe_item_menu_item m;
    swipe_item_menu_item_handler<platform::mock> h;

    h.map_text(m);
    h.map_icon_uri(m);
    h.clear_calls();

    m.text     = "Delete";
    m.icon_uri = "trash.png";

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "text");
    CHECK(h.calls()[0].value_repr    == "Delete");
    CHECK(h.calls()[1].property_name == "icon_uri");
    CHECK(h.calls()[1].value_repr    == "trash.png");

    m.text = "Delete";          // suppressed by Observable de-dup
    REQUIRE(h.calls().size() == 2);
}

TEST_CASE("swipe_item_menu_item mock records invoked signal emissions",
          "[mock][swipe_item_menu_item][invoked]") {
    swipe_item_menu_item m;
    swipe_item_menu_item_handler<platform::mock> h;

    h.map_invoked(m);
    h.clear_calls();

    m.invoked.emit();
    m.invoked.emit();

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "invoked");
    CHECK(h.calls()[0].has_value     == false);
    CHECK(h.calls()[1].property_name == "invoked");
}
