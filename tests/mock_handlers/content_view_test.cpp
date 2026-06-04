// SPDX-License-Identifier: Apache-2.0
#include <memory>

#include <catch2/catch_test_macros.hpp>
#include <mpapp/internal/basic_content_view.hpp>
#include <mpapp/handlers/mock/content_view_handler.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {
struct cv_child : view {
    cv_child() = default;
};
} // namespace

TEST_CASE("content_view mock records initial content as null",
          "[mock][content_view]") {
    internal::basic_content_view c;
    internal::content_view_handler<platform::mock> h;
    h.map_content(c);
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "content");
    CHECK(h.calls()[0].value_repr    == "null");
}

TEST_CASE("content_view mock records content changes after bind",
          "[mock][content_view]") {
    internal::basic_content_view c;
    internal::content_view_handler<platform::mock> h;
    h.map_content(c);
    h.clear_calls();

    c.content = std::make_shared<cv_child>();          // -> "set"
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "content");
    CHECK(h.calls()[0].value_repr    == "set");

    c.content = std::shared_ptr<view>{};               // -> "null"
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "null");
}
