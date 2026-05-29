// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_refresh_view`.

#include <catch2/catch_test_macros.hpp>

#include <memory>

#include <mpapp/handlers/mock/refresh_view_handler.hpp>
#include <mpapp/internal/basic_refresh_view.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

class plain_view : public view {};

} // namespace

TEST_CASE("refresh_view mock records initial values on bind",
          "[mock][refresh_view]") {
    internal::basic_refresh_view rv;
    internal::refresh_view_handler<platform::mock> h;

    h.map_content(rv);
    h.map_is_refreshing(rv);
    h.map_refresh_color(rv);

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "content.present");
    CHECK(h.calls()[0].value_repr    == "false");
    CHECK(h.calls()[1].property_name == "is_refreshing");
    CHECK(h.calls()[1].value_repr    == "false");
    CHECK(h.calls()[2].property_name == "refresh_color");
}

TEST_CASE("refresh_view mock records is_refreshing transitions",
          "[mock][refresh_view]") {
    internal::basic_refresh_view rv;
    internal::refresh_view_handler<platform::mock> h;

    h.map_is_refreshing(rv);
    h.clear_calls();

    rv.is_refreshing = true;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "true");

    rv.is_refreshing = true;             // idempotent — no extra row
    REQUIRE(h.calls().size() == 1);

    rv.is_refreshing = false;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "false");
}

TEST_CASE("refresh_view mock tracks content presence and color changes",
          "[mock][refresh_view][content]") {
    internal::basic_refresh_view rv;
    internal::refresh_view_handler<platform::mock> h;

    h.map_content(rv);
    h.map_refresh_color(rv);
    h.clear_calls();

    rv.content = std::make_shared<plain_view>();
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "content.present");
    CHECK(h.calls()[0].value_repr    == "true");

    rv.refresh_color = brush_ref{"DodgerBlue"};
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].property_name == "refresh_color");
    CHECK(h.calls()[1].value_repr    == "DodgerBlue");
}
