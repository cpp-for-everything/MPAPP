// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_shape_view`.

#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/shape_view_handler.hpp>
#include <mpapp/shape_view.hpp>

using namespace mpapp;

TEST_CASE("shape_view defaults",
          "[mock][shape_view]") {
    internal::basic_shape_view sv;
    CHECK(sv.kind.get()             == shape_kind::rectangle);
    CHECK(sv.data.get().empty());
    CHECK(sv.fill.get().empty());
    CHECK(sv.stroke.get().empty());
    CHECK(sv.stroke_thickness.get() == 1.0);
    CHECK(sv.opacity.get()          == 1.0);
}

TEST_CASE("shape_view mock records kind + data + fill changes",
          "[mock][shape_view]") {
    internal::basic_shape_view sv;
    shape_view_handler<platform::mock> h;
    h.map_kind(sv);
    h.map_data(sv);
    h.map_fill(sv);
    h.clear_calls();

    sv.kind = shape_kind::path;
    sv.data = "M0 0 L10 10";
    sv.fill = "#ff00ff";

    REQUIRE(h.calls_as_strings().size() == 3);
    CHECK(h.calls_as_strings()[0] == "kind=4");          // shape_kind::path == 4
    CHECK(h.calls_as_strings()[1] == "data=M0 0 L10 10");
    CHECK(h.calls_as_strings()[2] == "fill=#ff00ff");
}
