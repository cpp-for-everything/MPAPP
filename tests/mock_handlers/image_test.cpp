// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_image`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/image_handler.hpp>
#include <mpapp/internal/basic_image.hpp>

using namespace mpapp;

TEST_CASE("image mock records initial values on bind",
          "[mock][image]") {
    internal::basic_image i;
    internal::image_handler<platform::mock> h;

    h.map_source(i);
    h.map_aspect(i);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "source");
    CHECK(h.calls()[0].value_repr    == "");
    CHECK(h.calls()[1].property_name == "aspect");
    CHECK(h.calls()[1].value_repr    == "0");
}

TEST_CASE("image records source changes",
          "[mock][image]") {
    internal::basic_image i;
    internal::image_handler<platform::mock> h;

    h.map_source(i);
    h.clear_calls();

    i.source = "icon.png";
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "icon.png");
}

TEST_CASE("image records aspect changes",
          "[mock][image]") {
    internal::basic_image i;
    internal::image_handler<platform::mock> h;

    h.map_aspect(i);
    h.clear_calls();

    i.aspect = aspect_mode::center;     // 3
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "aspect");
    CHECK(h.calls()[0].value_repr    == "3");
}
