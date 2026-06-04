// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::internal::basic_absolute_layout`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_absolute_layout.hpp>
#include <mpapp/handlers/mock/absolute_layout_handler.hpp>

using namespace mpapp;

TEST_CASE("absolute_layout_flags to_string covers every enumerator",
          "[mock][absolute_layout]") {
    CHECK(to_string(absolute_layout_flags::none)                  == "none");
    CHECK(to_string(absolute_layout_flags::x_proportional)        == "x_proportional");
    CHECK(to_string(absolute_layout_flags::y_proportional)        == "y_proportional");
    CHECK(to_string(absolute_layout_flags::width_proportional)    == "width_proportional");
    CHECK(to_string(absolute_layout_flags::height_proportional)   == "height_proportional");
    CHECK(to_string(absolute_layout_flags::position_proportional) == "position_proportional");
    CHECK(to_string(absolute_layout_flags::size_proportional)     == "size_proportional");
    CHECK(to_string(absolute_layout_flags::all)                   == "all");

    // Unmapped bit-combination falls through to the sentinel.
    CHECK(to_string(static_cast<absolute_layout_flags>(0x55)) == "?");
}

TEST_CASE("absolute_layout_flags numeric values match MAUI",
          "[mock][absolute_layout]") {
    CHECK(static_cast<int>(absolute_layout_flags::none)                  == 0);
    CHECK(static_cast<int>(absolute_layout_flags::x_proportional)        == 1);
    CHECK(static_cast<int>(absolute_layout_flags::y_proportional)        == 2);
    CHECK(static_cast<int>(absolute_layout_flags::width_proportional)    == 4);
    CHECK(static_cast<int>(absolute_layout_flags::height_proportional)   == 8);
    CHECK(static_cast<int>(absolute_layout_flags::position_proportional) == 3);
    CHECK(static_cast<int>(absolute_layout_flags::size_proportional)     == 12);
    CHECK(static_cast<int>(absolute_layout_flags::all)                   == 15);
}

TEST_CASE("rect default-constructs to zero and compares by value",
          "[mock][absolute_layout]") {
    rect a;
    CHECK(a.x == 0.0);
    CHECK(a.y == 0.0);
    CHECK(a.width == 0.0);
    CHECK(a.height == 0.0);

    rect b{1.0, 2.0, 3.0, 4.0};
    rect c{1.0, 2.0, 3.0, 4.0};
    CHECK(a == rect{});
    CHECK(b == c);
    CHECK_FALSE(a == b);
}

TEST_CASE("get_layout_bounds / get_layout_flags return defaults for unknown child",
          "[mock][absolute_layout]") {
    internal::basic_absolute_layout a;
    view child;

    // Arrange: nothing attached yet.
    // Act + Assert: defaults come back.
    CHECK(a.get_layout_bounds(child) == rect{});
    CHECK(a.get_layout_flags(child) == absolute_layout_flags::none);
}

TEST_CASE("set_layout_bounds / set_layout_flags round-trip through the attached store",
          "[mock][absolute_layout]") {
    internal::basic_absolute_layout a;
    view child;

    // Act.
    a.set_layout_bounds(child, rect{10.0, 20.0, 30.0, 40.0});
    a.set_layout_flags(child, absolute_layout_flags::all);

    // Assert.
    CHECK(a.get_layout_bounds(child) == rect{10.0, 20.0, 30.0, 40.0});
    CHECK(a.get_layout_flags(child) == absolute_layout_flags::all);
}

TEST_CASE("attached store keys per child independently",
          "[mock][absolute_layout]") {
    internal::basic_absolute_layout a;
    view first;
    view second;

    a.set_layout_bounds(first, rect{1.0, 1.0, 1.0, 1.0});
    a.set_layout_flags(first, absolute_layout_flags::position_proportional);
    a.set_layout_bounds(second, rect{2.0, 2.0, 2.0, 2.0});

    CHECK(a.get_layout_bounds(first) == rect{1.0, 1.0, 1.0, 1.0});
    CHECK(a.get_layout_flags(first) == absolute_layout_flags::position_proportional);
    CHECK(a.get_layout_bounds(second) == rect{2.0, 2.0, 2.0, 2.0});
    // second never had flags set - stays at the default.
    CHECK(a.get_layout_flags(second) == absolute_layout_flags::none);
}

TEST_CASE("setting bounds then flags on same child preserves both",
          "[mock][absolute_layout]") {
    internal::basic_absolute_layout a;
    view child;

    a.set_layout_bounds(child, rect{5.0, 6.0, 7.0, 8.0});
    a.set_layout_flags(child, absolute_layout_flags::size_proportional);
    // Re-set bounds; flags must survive.
    a.set_layout_bounds(child, rect{9.0, 10.0, 11.0, 12.0});

    CHECK(a.get_layout_bounds(child) == rect{9.0, 10.0, 11.0, 12.0});
    CHECK(a.get_layout_flags(child) == absolute_layout_flags::size_proportional);
}

TEST_CASE("has_handler is false on a freshly constructed surface",
          "[mock][absolute_layout]") {
    // The handler accessors are typed on platform::current (the real
    // per-platform handler), so the surface test can only assert the
    // unbound default - binding a real handler needs the native SDK and
    // lives in the wrapper. Mirrors grid_layout's surface test scope.
    internal::basic_absolute_layout a;
    CHECK_FALSE(a.has_handler());
}

TEST_CASE("mock handler records the child's current attached values",
          "[mock][absolute_layout]") {
    internal::basic_absolute_layout a;
    internal::absolute_layout_handler<platform::mock> h;
    view child;

    a.set_layout_bounds(child, rect{0.0, 50.0, 0.5, 100.0});
    a.set_layout_flags(child, absolute_layout_flags::all);

    h.map_layout_bounds(a, child);
    h.map_layout_flags(a, child);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "layout_bounds");
    CHECK(h.calls()[0].value_repr    == "rect(0,50,0.5,100)");
    CHECK(h.calls()[1].property_name == "layout_flags");
    CHECK(h.calls()[1].value_repr    == "all");
}

TEST_CASE("mock handler records defaults for an unattached child",
          "[mock][absolute_layout]") {
    internal::basic_absolute_layout a;
    internal::absolute_layout_handler<platform::mock> h;
    view child;

    h.map_layout_bounds(a, child);
    h.map_layout_flags(a, child);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "layout_bounds=rect(0,0,0,0)",
        "layout_flags=none",
    });
}

TEST_CASE("mock handler inherits the layout child-list command mappers",
          "[mock][absolute_layout]") {
    internal::basic_absolute_layout a;
    internal::absolute_layout_handler<platform::mock> h;
    view child;

    h.map_add(a, child);
    h.map_insert(a, 2, child);
    h.map_remove(a, child);
    h.map_update_z_index(a, child, 7);
    h.map_clear(a);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "add",
        "insert=2",
        "remove",
        "update_z_index=7",
        "clear",
    });
}

TEST_CASE("mock handler inherits the base layout property mappers",
          "[mock][absolute_layout]") {
    internal::basic_absolute_layout a;
    internal::absolute_layout_handler<platform::mock> h;

    h.map_padding(a);
    h.map_is_clipped_to_bounds(a);
    h.map_cascade_input_transparent(a);
    h.clear_calls();

    // Setting a bound layout property records once per real change.
    a.is_clipped_to_bounds = true;
    a.cascade_input_transparent = true;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "is_clipped_to_bounds=true",
        "cascade_input_transparent=true",
    });
}

TEST_CASE("map_gestures is a no-op stub that records nothing",
          "[mock][absolute_layout]") {
    internal::basic_absolute_layout a;
    internal::absolute_layout_handler<platform::mock> h;

    h.map_gestures(a);

    CHECK(h.calls().empty());
}
