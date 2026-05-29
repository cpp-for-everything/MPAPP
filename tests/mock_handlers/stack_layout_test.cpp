// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::internal::basic_stack_layout` (T-0011).

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_button.hpp>
#include <mpapp/handlers/mock/stack_layout_handler.hpp>
#include <mpapp/internal/basic_stack_layout.hpp>

using namespace mpapp;

TEST_CASE("stack_layout mock handler records initial property values on bind",
          "[mock][stack_layout]") {
    internal::basic_stack_layout s;
    internal::stack_layout_handler<platform::mock> h;

    h.map_orientation(s);
    h.map_spacing(s);
    h.map_horizontal_alignment(s);
    h.map_vertical_alignment(s);

    REQUIRE(h.calls().size() == 4);
    CHECK(h.calls()[0].property_name == "orientation");
    CHECK(h.calls()[0].value_repr    == "vertical");
    CHECK(h.calls()[1].property_name == "spacing");
    CHECK(h.calls()[1].value_repr    == "0");
    CHECK(h.calls()[2].property_name == "horizontal_alignment");
    CHECK(h.calls()[2].value_repr    == "stretch");
    CHECK(h.calls()[3].property_name == "vertical_alignment");
    CHECK(h.calls()[3].value_repr    == "stretch");
}

TEST_CASE("stack_layout mock handler fires once per real property change",
          "[mock][stack_layout]") {
    internal::basic_stack_layout s;
    internal::stack_layout_handler<platform::mock> h;

    h.map_orientation(s);
    h.map_spacing(s);
    h.map_horizontal_alignment(s);
    h.clear_calls();

    s.stack_orientation    = orientation::horizontal;
    s.stack_orientation    = orientation::horizontal;   // suppressed
    s.spacing              = 12.0;
    s.horizontal_alignment = h_align::center;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "orientation=horizontal",
        "spacing=12",
        "horizontal_alignment=center",
    });
}

TEST_CASE("stack_layout mock handler records child-mutation commands",
          "[mock][stack_layout]") {
    internal::basic_button b1, b2;
    internal::basic_stack_layout s;
    internal::stack_layout_handler<platform::mock> h;

    h.map_add(s, b1);
    h.map_add(s, b2);
    h.map_remove(s, b1);
    h.map_clear(s);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "add", "add", "remove", "clear",
    });
}
