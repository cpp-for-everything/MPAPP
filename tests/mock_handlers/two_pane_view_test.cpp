// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_two_pane_view` (ADR-0008).
//
// Covers: pane slot mutations, mode/priority/threshold observables, signal
// emissions, and idempotent-set behaviour.

#include <catch2/catch_test_macros.hpp>

#include <memory>

#include <mpapp/handlers/mock/two_pane_view_handler.hpp>
#include <mpapp/internal/basic_two_pane_view.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

struct dummy_view : view {
    dummy_view() = default;
};

} // anonymous namespace

// ---------------------------------------------------------------------------
// Pane slots
// ---------------------------------------------------------------------------

TEST_CASE("two_pane_view mock records initial pane1 as absent",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_pane1(v);
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "pane1.present");
    CHECK(h.calls()[0].value_repr    == "false");
}

TEST_CASE("two_pane_view mock records initial pane2 as absent",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_pane2(v);
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "pane2.present");
    CHECK(h.calls()[0].value_repr    == "false");
}

TEST_CASE("two_pane_view mock records pane1 set and clear",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_pane1(v);
    h.clear_calls();

    v.set_pane1(std::make_shared<dummy_view>());
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "pane1.present");
    CHECK(h.calls()[0].value_repr    == "true");

    v.set_pane1(nullptr);
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "false");
}

TEST_CASE("two_pane_view mock records pane2 set and clear",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_pane2(v);
    h.clear_calls();

    v.set_pane2(std::make_shared<dummy_view>());
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "pane2.present");
    CHECK(h.calls()[0].value_repr    == "true");

    v.set_pane2(nullptr);
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "false");
}

TEST_CASE("two_pane_view pane1 idempotent set does not fire signal",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_pane1(v);
    h.clear_calls();

    // Setting nullptr on an already-null slot must not fire.
    v.set_pane1(nullptr);
    CHECK(h.calls().empty());

    auto child = std::make_shared<dummy_view>();
    v.set_pane1(child);
    h.clear_calls();

    // Setting the same pointer again must not fire.
    v.set_pane1(child);
    CHECK(h.calls().empty());
}

TEST_CASE("two_pane_view pane2 idempotent set does not fire signal",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_pane2(v);
    h.clear_calls();

    v.set_pane2(nullptr);
    CHECK(h.calls().empty());
}

// ---------------------------------------------------------------------------
// mode property
// ---------------------------------------------------------------------------

TEST_CASE("two_pane_view mock records initial mode as single_pane",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_mode(v);
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "mode");
    CHECK(h.calls()[0].value_repr    == "single_pane");
}

TEST_CASE("two_pane_view mock records mode transitions",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_mode(v);
    h.clear_calls();

    v.mode = two_pane_mode::wide;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "wide");

    v.mode = two_pane_mode::tall;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "tall");

    v.mode = two_pane_mode::single_pane;
    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[2].value_repr == "single_pane");
}

TEST_CASE("two_pane_view mode idempotent set does not record again",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_mode(v);
    h.clear_calls();

    v.mode = two_pane_mode::single_pane; // same as default
    CHECK(h.calls().empty());
}

// ---------------------------------------------------------------------------
// panel_priority property
// ---------------------------------------------------------------------------

TEST_CASE("two_pane_view mock records initial panel_priority as pane1",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_panel_priority(v);
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "panel_priority");
    CHECK(h.calls()[0].value_repr    == "pane1");
}

TEST_CASE("two_pane_view mock records panel_priority change to pane2",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_panel_priority(v);
    h.clear_calls();

    v.panel_priority = two_pane_priority::pane2;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "pane2");

    v.panel_priority = two_pane_priority::pane1;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "pane1");
}

TEST_CASE("two_pane_view panel_priority idempotent set does not record again",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_panel_priority(v);
    h.clear_calls();

    v.panel_priority = two_pane_priority::pane1; // same as default
    CHECK(h.calls().empty());
}

// ---------------------------------------------------------------------------
// min_wide_mode_width property
// ---------------------------------------------------------------------------

TEST_CASE("two_pane_view mock records initial min_wide_mode_width",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_min_wide_mode_width(v);
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "min_wide_mode_width");
    CHECK(h.calls()[0].value_repr    == "641");
}

TEST_CASE("two_pane_view mock records min_wide_mode_width change",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_min_wide_mode_width(v);
    h.clear_calls();

    v.min_wide_mode_width = 800.0;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "800");
}

// ---------------------------------------------------------------------------
// min_tall_mode_height property
// ---------------------------------------------------------------------------

TEST_CASE("two_pane_view mock records initial min_tall_mode_height",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_min_tall_mode_height(v);
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "min_tall_mode_height");
    CHECK(h.calls()[0].value_repr    == "641");
}

TEST_CASE("two_pane_view mock records min_tall_mode_height change",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;
    h.map_min_tall_mode_height(v);
    h.clear_calls();

    v.min_tall_mode_height = 1024.0;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "1024");
}

// ---------------------------------------------------------------------------
// to_string / enum coverage
// ---------------------------------------------------------------------------

TEST_CASE("two_pane_mode to_string covers all enumerators",
          "[mock][two_pane_view][enum]") {
    CHECK(to_string(two_pane_mode::single_pane) == "single_pane");
    CHECK(to_string(two_pane_mode::wide)        == "wide");
    CHECK(to_string(two_pane_mode::tall)        == "tall");
}

TEST_CASE("two_pane_priority to_string covers all enumerators",
          "[mock][two_pane_view][enum]") {
    CHECK(to_string(two_pane_priority::pane1) == "pane1");
    CHECK(to_string(two_pane_priority::pane2) == "pane2");
}

// ---------------------------------------------------------------------------
// pane1/pane2 accessors (without handler)
// ---------------------------------------------------------------------------

TEST_CASE("two_pane_view pane accessors return nullptr by default",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    CHECK(v.pane1() == nullptr);
    CHECK(v.pane2() == nullptr);
}

TEST_CASE("two_pane_view set_pane1 and pane1 accessor round-trip",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    auto child = std::make_shared<dummy_view>();
    v.set_pane1(child);
    CHECK(v.pane1() == child);
    v.set_pane1(nullptr);
    CHECK(v.pane1() == nullptr);
}

TEST_CASE("two_pane_view set_pane2 and pane2 accessor round-trip",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    auto child = std::make_shared<dummy_view>();
    v.set_pane2(child);
    CHECK(v.pane2() == child);
    v.set_pane2(nullptr);
    CHECK(v.pane2() == nullptr);
}

// ---------------------------------------------------------------------------
// has_handler (set_handler takes platform::current; only the unbound
// default is exercisable in mock-surface tests -- mirrors absolute_layout)
// ---------------------------------------------------------------------------

TEST_CASE("two_pane_view has_handler is false by default",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    CHECK_FALSE(v.has_handler());
}

// ---------------------------------------------------------------------------
// Multiple mappers together
// ---------------------------------------------------------------------------

TEST_CASE("two_pane_view all mappers record initial values",
          "[mock][two_pane_view]") {
    internal::basic_two_pane_view v;
    internal::two_pane_view_handler<platform::mock> h;

    h.map_pane1(v);
    h.map_pane2(v);
    h.map_mode(v);
    h.map_panel_priority(v);
    h.map_min_wide_mode_width(v);
    h.map_min_tall_mode_height(v);

    // 6 initial records: pane1.present, pane2.present, mode,
    //                    panel_priority, min_wide_mode_width, min_tall_mode_height
    REQUIRE(h.calls().size() == 6);

    CHECK(h.calls()[0].property_name == "pane1.present");
    CHECK(h.calls()[0].value_repr    == "false");

    CHECK(h.calls()[1].property_name == "pane2.present");
    CHECK(h.calls()[1].value_repr    == "false");

    CHECK(h.calls()[2].property_name == "mode");
    CHECK(h.calls()[2].value_repr    == "single_pane");

    CHECK(h.calls()[3].property_name == "panel_priority");
    CHECK(h.calls()[3].value_repr    == "pane1");

    CHECK(h.calls()[4].property_name == "min_wide_mode_width");
    CHECK(h.calls()[5].property_name == "min_tall_mode_height");
}
