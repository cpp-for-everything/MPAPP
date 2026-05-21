// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::templated_view` (M-04b).

#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/templated_view_handler.hpp>
#include <mpapp/templated_view.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

class plain_view : public view {};

} // namespace

TEST_CASE("templated_view mock handler records initial property values on bind",
          "[mock][templated_view]") {
    templated_view t;
    templated_view_handler<platform::mock> h;

    h.map_content(t);
    h.map_template_id(t);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "content");
    CHECK(h.calls()[0].value_repr    == "null");
    CHECK(h.calls()[1].property_name == "template_id");
    CHECK(h.calls()[1].value_repr    == "");
}

TEST_CASE("templated_view mock handler tracks content + template_id changes",
          "[mock][templated_view]") {
    templated_view t;
    templated_view_handler<platform::mock> h;

    h.map_content(t);
    h.map_template_id(t);
    h.clear_calls();

    t.content     = std::make_shared<plain_view>();
    t.template_id = "card-template";
    t.template_id = "card-template";   // idempotent — no record
    t.content     = nullptr;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "content=set",
        "template_id=card-template",
        "content=null",
    });
}
