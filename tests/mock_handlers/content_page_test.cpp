// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::internal::basic_content_page` (M-04b).

#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_content_page.hpp>
#include <mpapp/handlers/mock/content_page_handler.hpp>
#include <mpapp/layout.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

class plain_view : public view {};

} // namespace

TEST_CASE("content_page mock handler records initial property values on bind",
          "[mock][content_page]") {
    internal::basic_content_page p;
    internal::content_page_handler<platform::mock> h;

    h.map_title(p);
    h.map_content(p);
    h.map_padding(p);

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "title");
    CHECK(h.calls()[0].value_repr    == "");
    CHECK(h.calls()[1].property_name == "content");
    CHECK(h.calls()[1].value_repr    == "null");
    CHECK(h.calls()[2].property_name == "padding");
    CHECK(h.calls()[2].value_repr    == "thickness(0,0,0,0)");
}

TEST_CASE("content_page mock handler tracks title + content + padding changes",
          "[mock][content_page]") {
    internal::basic_content_page p;
    internal::content_page_handler<platform::mock> h;

    h.map_title(p);
    h.map_content(p);
    h.map_padding(p);
    h.clear_calls();

    p.title   = "Home";
    p.content = std::make_shared<plain_view>();
    p.padding = thickness{12.0, 8.0, 12.0, 8.0};
    p.padding = thickness{12.0, 8.0, 12.0, 8.0};  // suppressed (idempotent)
    p.content = nullptr;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "title=Home",
        "content=set",
        "padding=thickness(12,8,12,8)",
        "content=null",
    });
}

TEST_CASE("content_page mock handler records single call per property change",
          "[mock][content_page]") {
    internal::basic_content_page p;
    internal::content_page_handler<platform::mock> h;

    h.map_padding(p);
    h.clear_calls();

    p.padding = thickness{4.0};
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "thickness(4,4,4,4)");

    p.padding = thickness{4.0};       // idempotent - same value
    REQUIRE(h.calls().size() == 1);

    p.padding = thickness{6.0};
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "thickness(6,6,6,6)");
}
