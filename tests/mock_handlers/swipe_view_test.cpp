// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_swipe_view`.

#include <catch2/catch_test_macros.hpp>

#include <vector>

#include <mpapp/handlers/mock/swipe_view_handler.hpp>
#include <mpapp/internal/basic_swipe_view.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

class plain_view : public view {};

} // namespace

TEST_CASE("swipe_view mock records initial values on bind",
          "[mock][swipe_view]") {
    internal::basic_swipe_view sv;
    internal::swipe_view_handler<platform::mock> h;

    h.map_content(sv);
    h.map_left_items(sv);
    h.map_right_items(sv);

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "content.present");
    CHECK(h.calls()[0].value_repr    == "false");
    CHECK(h.calls()[1].property_name == "left_items.count");
    CHECK(h.calls()[1].value_repr    == "0");
    CHECK(h.calls()[2].property_name == "right_items.count");
    CHECK(h.calls()[2].value_repr    == "0");
}

TEST_CASE("swipe_view mock tracks content presence changes",
          "[mock][swipe_view]") {
    plain_view child;
    internal::basic_swipe_view sv;
    internal::swipe_view_handler<platform::mock> h;

    h.map_content(sv);
    h.clear_calls();

    sv.content = &child;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "content.present");
    CHECK(h.calls()[0].value_repr    == "true");

    sv.content = &child;     // suppressed — Observable de-dupes
    REQUIRE(h.calls().size() == 1);

    sv.content = nullptr;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "false");
}

TEST_CASE("swipe_view mock records left/right item count changes",
          "[mock][swipe_view][items]") {
    plain_view a, b;
    internal::basic_swipe_view sv;
    internal::swipe_view_handler<platform::mock> h;

    h.map_left_items(sv);
    h.map_right_items(sv);
    h.clear_calls();

    sv.left_items  = std::vector<view*>{&a};
    sv.right_items = std::vector<view*>{&a, &b};

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "left_items.count");
    CHECK(h.calls()[0].value_repr    == "1");
    CHECK(h.calls()[1].property_name == "right_items.count");
    CHECK(h.calls()[1].value_repr    == "2");

    sv.left_items = std::vector<view*>{};
    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[2].value_repr == "0");
}
