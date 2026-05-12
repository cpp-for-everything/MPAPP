// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Layout.md
//
// Mock-handler tests for `mpapp::layout` (CLAUDE Rule 6 / ADR-0008).

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/layout_handler.hpp>
#include <mpapp/layout.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

// `layout` is abstract in the vault spec; the mock-test layer subclasses
// it directly to instantiate. The subclass adds no behaviour — it exists
// only to allow construction. A real `create_layout_manager()` lives
// with the concrete strategy subclasses (grid, stack, …) and is not
// needed for the mock contract.
class test_layout : public layout {};

} // namespace

TEST_CASE("layout mock handler records initial property values on bind",
          "[mock][layout]") {
    test_layout l;
    layout_handler<platform::mock> h;

    h.map_padding(l);
    h.map_is_clipped_to_bounds(l);
    h.map_cascade_input_transparent(l);

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "padding");
    CHECK(h.calls()[0].value_repr    == "thickness(0,0,0,0)");
    CHECK(h.calls()[1].property_name == "is_clipped_to_bounds");
    CHECK(h.calls()[1].value_repr    == "false");
    CHECK(h.calls()[2].property_name == "cascade_input_transparent");
}

TEST_CASE("layout mock handler records single call per property change",
          "[mock][layout]") {
    test_layout l;
    layout_handler<platform::mock> h;

    h.map_padding(l);
    h.clear_calls();

    l.padding = thickness{8.0};
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "padding");
    CHECK(h.calls()[0].value_repr    == "thickness(8,8,8,8)");

    // Idempotent: re-setting the same thickness fires nothing.
    l.padding = thickness{8.0};
    REQUIRE(h.calls().size() == 1);
}

TEST_CASE("layout mock handler records child command sequence",
          "[mock][layout][commands]") {
    test_layout l;
    view a;
    view b;
    layout_handler<platform::mock> h;

    h.map_add(l, a);
    h.map_add(l, b);
    h.map_insert(l, 0, b);
    h.map_remove(l, a);
    h.map_update_z_index(l, b, 5);
    h.map_clear(l);

    REQUIRE(h.calls().size() == 6);
    CHECK(h.calls()[0].property_name == "add");
    CHECK(h.calls()[1].property_name == "add");
    CHECK(h.calls()[2].property_name == "insert");
    CHECK(h.calls()[2].value_repr    == "0");
    CHECK(h.calls()[3].property_name == "remove");
    CHECK(h.calls()[4].property_name == "update_z_index");
    CHECK(h.calls()[4].value_repr    == "5");
    CHECK(h.calls()[5].property_name == "clear");
}

TEST_CASE("layout inline mutators update the children vector",
          "[mock][layout][storage]") {
    test_layout l;
    view a;
    view b;
    view c;

    REQUIRE(l.child_count() == 0);

    l.add(a);
    l.add(b);
    REQUIRE(l.child_count() == 2);
    CHECK(l.child_at(0) == &a);
    CHECK(l.child_at(1) == &b);

    l.insert(1, c);
    REQUIRE(l.child_count() == 3);
    CHECK(l.child_at(1) == &c);

    l.remove(a);
    REQUIRE(l.child_count() == 2);
    CHECK(l.child_at(0) == &c);

    l.clear();
    REQUIRE(l.child_count() == 0);
}
