// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::swipe_item_view`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/swipe_item_view_handler.hpp>
#include <mpapp/swipe_item_view.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

class plain_view : public view {};

} // namespace

TEST_CASE("swipe_item_view mock records initial values on bind",
          "[mock][swipe_item_view]") {
    swipe_item_view iv;
    swipe_item_view_handler<platform::mock> h;

    h.map_content(iv);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "content.present");
    CHECK(h.calls()[0].value_repr    == "false");
}

TEST_CASE("swipe_item_view mock tracks content presence transitions",
          "[mock][swipe_item_view]") {
    plain_view child;
    swipe_item_view iv;
    swipe_item_view_handler<platform::mock> h;

    h.map_content(iv);
    h.clear_calls();

    iv.content = &child;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "true");

    iv.content = &child;          // suppressed
    REQUIRE(h.calls().size() == 1);

    iv.content = nullptr;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "false");
}
