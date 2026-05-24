// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_menu_bar_item`.

#include <catch2/catch_test_macros.hpp>

#include <vector>

#include <mpapp/handlers/mock/menu_bar_item_handler.hpp>
#include <mpapp/menu_bar_item.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

class plain_view : public view {};

} // namespace

TEST_CASE("menu_bar_item mock records initial values on bind",
          "[mock][menu_bar_item]") {
    internal::basic_menu_bar_item m;
    menu_bar_item_handler<platform::mock> h;

    h.map_title(m);
    h.map_items(m);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "title");
    CHECK(h.calls()[0].value_repr    == "");
    CHECK(h.calls()[1].property_name == "items.count");
    CHECK(h.calls()[1].value_repr    == "0");
}

TEST_CASE("menu_bar_item records title and items changes",
          "[mock][menu_bar_item]") {
    internal::basic_menu_bar_item m;
    menu_bar_item_handler<platform::mock> h;

    h.map_title(m);
    h.map_items(m);
    h.clear_calls();

    m.title = "File";
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "title");
    CHECK(h.calls()[0].value_repr    == "File");

    m.title = "File";          // idempotent — no re-emit on equal set
    REQUIRE(h.calls().size() == 1);

    plain_view a, b;
    m.items = std::vector<view*>{ &a, &b };
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].property_name == "items.count");
    CHECK(h.calls()[1].value_repr    == "2");
}
