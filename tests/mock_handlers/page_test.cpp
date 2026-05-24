// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::internal::basic_page` (T-0011).

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/button.hpp>
#include <mpapp/handlers/mock/page_handler.hpp>
#include <mpapp/page.hpp>

using namespace mpapp;

TEST_CASE("page mock handler records initial property values on bind",
          "[mock][page]") {
    internal::basic_page p;
    page_handler<platform::mock> h;

    h.map_title(p);
    h.map_content(p);
    h.map_is_busy(p);

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "title");
    CHECK(h.calls()[1].property_name == "content.present");
    CHECK(h.calls()[1].value_repr    == "false");
    CHECK(h.calls()[2].property_name == "is_busy");
    CHECK(h.calls()[2].value_repr    == "false");
}

TEST_CASE("page mock handler tracks title + content + is_busy changes",
          "[mock][page]") {
    internal::basic_button child;
    internal::basic_page p;
    page_handler<platform::mock> h;

    h.map_title(p);
    h.map_content(p);
    h.map_is_busy(p);
    h.clear_calls();

    p.title    = "Home";
    p.content  = &child;
    p.is_busy  = true;
    p.is_busy  = true;          // suppressed
    p.is_busy  = false;
    p.content  = nullptr;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "title=Home",
        "content.present=true",
        "is_busy=true",
        "is_busy=false",
        "content.present=false",
    });
}
