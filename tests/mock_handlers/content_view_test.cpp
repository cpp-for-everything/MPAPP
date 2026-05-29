// SPDX-License-Identifier: Apache-2.0
#include <catch2/catch_test_macros.hpp>
#include <mpapp/internal/basic_content_view.hpp>
#include <mpapp/handlers/mock/content_view_handler.hpp>

using namespace mpapp;

TEST_CASE("content_view mock records initial content as null",
          "[mock][content_view]") {
    internal::basic_content_view c;
    internal::content_view_handler<platform::mock> h;
    h.map_content(c);
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "content");
    CHECK(h.calls()[0].value_repr    == "null");
}
