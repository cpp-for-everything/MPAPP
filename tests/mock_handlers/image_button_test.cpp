// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_image_button`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/image_button_handler.hpp>
#include <mpapp/internal/basic_image_button.hpp>

using namespace mpapp;

TEST_CASE("image_button mock records initial values on bind",
          "[mock][image_button]") {
    internal::basic_image_button b;
    internal::image_button_handler<platform::mock> h;

    h.map_source(b);
    h.map_aspect(b);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "source");
    CHECK(h.calls()[1].property_name == "aspect");
}

TEST_CASE("image_button records aspect changes after bind",
          "[mock][image_button]") {
    internal::basic_image_button b;
    internal::image_button_handler<platform::mock> h;
    h.map_aspect(b);
    h.clear_calls();

    b.aspect = aspect_mode::fill;       // 2
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "aspect");
    CHECK(h.calls()[0].value_repr    == "2");
}
