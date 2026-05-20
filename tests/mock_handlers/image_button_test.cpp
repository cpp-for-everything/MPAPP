// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::image_button`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/image_button_handler.hpp>
#include <mpapp/image_button.hpp>

using namespace mpapp;

TEST_CASE("image_button mock records initial values on bind",
          "[mock][image_button]") {
    image_button b;
    image_button_handler<platform::mock> h;

    h.map_source(b);
    h.map_aspect(b);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "source");
    CHECK(h.calls()[1].property_name == "aspect");
}
