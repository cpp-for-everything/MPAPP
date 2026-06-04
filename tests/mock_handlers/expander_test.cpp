// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_expander` (ADR-0008).
//
// All tests use only the platform-agnostic surface + the mock handler so the
// translation unit stays link-isolated from any real platform backend.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/expander_handler.hpp>
#include <mpapp/internal/basic_expander.hpp>

using namespace mpapp;
using expander_mock = internal::expander_handler<platform::mock>;

// ---------------------------------------------------------------------------
// Helper views used as header / content slots
// ---------------------------------------------------------------------------
namespace {

class stub_view : public view {};

} // namespace

// ---------------------------------------------------------------------------
// is_expanded property
// ---------------------------------------------------------------------------

TEST_CASE("expander mock records initial is_expanded as false",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    h.map_is_expanded(e);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "is_expanded");
    CHECK(h.calls()[0].value_repr    == "false");
}

TEST_CASE("expander mock records is_expanded change to true",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    h.map_is_expanded(e);
    h.clear_calls();

    e.is_expanded = true;

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "is_expanded");
    CHECK(h.calls()[0].value_repr    == "true");
}

TEST_CASE("expander mock does not record same-value is_expanded write",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    h.map_is_expanded(e);
    h.clear_calls();

    e.is_expanded = false;  // same as default

    CHECK(h.calls().empty());
}

TEST_CASE("expander mock records toggle sequence for is_expanded",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    h.map_is_expanded(e);
    h.clear_calls();

    e.is_expanded = true;
    e.is_expanded = false;
    e.is_expanded = true;

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].value_repr == "true");
    CHECK(h.calls()[1].value_repr == "false");
    CHECK(h.calls()[2].value_repr == "true");
}

// ---------------------------------------------------------------------------
// direction property
// ---------------------------------------------------------------------------

TEST_CASE("expander mock records initial direction as down",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    h.map_direction(e);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "direction");
    CHECK(h.calls()[0].value_repr    == "down");
}

TEST_CASE("expander mock records direction change to up",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    h.map_direction(e);
    h.clear_calls();

    e.direction = expand_direction::up;

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "up");
}

TEST_CASE("expander mock does not record same-value direction write",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    h.map_direction(e);
    h.clear_calls();

    e.direction = expand_direction::down;  // same as default

    CHECK(h.calls().empty());
}

// ---------------------------------------------------------------------------
// header slot
// ---------------------------------------------------------------------------

TEST_CASE("expander mock records header as null when unset",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    h.map_header(e);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "header");
    CHECK(h.calls()[0].value_repr    == "null");
}

TEST_CASE("expander mock records header as set after set_header",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    stub_view sv;
    e.set_header(&sv);

    h.map_header(e);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "set");
}

// ---------------------------------------------------------------------------
// content slot
// ---------------------------------------------------------------------------

TEST_CASE("expander mock records content as null when unset",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    h.map_content(e);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "content");
    CHECK(h.calls()[0].value_repr    == "null");
}

TEST_CASE("expander mock records content as set after set_content",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    stub_view sv;
    e.set_content(&sv);

    h.map_content(e);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "set");
}

// ---------------------------------------------------------------------------
// expanded signal
// ---------------------------------------------------------------------------

TEST_CASE("expander mock records expanded signal on is_expanded true",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    h.map_expanded(e);

    e.is_expanded = true;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"expanded"});
}

TEST_CASE("expander mock records expanded twice on true-false-true cycle",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    h.map_expanded(e);

    e.is_expanded = true;
    e.is_expanded = false;
    e.is_expanded = true;

    // Only the true transitions produce "expanded".
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"expanded", "expanded"});
}

// ---------------------------------------------------------------------------
// collapsed signal
// ---------------------------------------------------------------------------

TEST_CASE("expander mock records collapsed signal on is_expanded false",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    // Start expanded so the first real change goes false -> collapsed.
    e.is_expanded = true;

    h.map_collapsed(e);
    h.clear_calls();

    e.is_expanded = false;

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"collapsed"});
}

TEST_CASE("expander mock records collapsed twice on false-true-false cycle",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    h.map_collapsed(e);

    e.is_expanded = true;
    e.is_expanded = false;
    e.is_expanded = true;
    e.is_expanded = false;

    // Only the false transitions produce "collapsed".
    REQUIRE(h.calls_as_strings() ==
            std::vector<std::string>{"collapsed", "collapsed"});
}

// ---------------------------------------------------------------------------
// expanded + collapsed wired together
// ---------------------------------------------------------------------------

TEST_CASE("expander mock records interleaved expanded and collapsed signals",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    h.map_expanded(e);
    h.map_collapsed(e);

    e.is_expanded = true;
    e.is_expanded = false;
    e.is_expanded = true;

    REQUIRE(h.calls_as_strings() ==
            std::vector<std::string>{"expanded", "collapsed", "expanded"});
}

// ---------------------------------------------------------------------------
// to_string for expand_direction
// ---------------------------------------------------------------------------

TEST_CASE("to_string for expand_direction returns correct labels",
          "[mock][expander]") {
    CHECK(to_string(expand_direction::down) == "down");
    CHECK(to_string(expand_direction::up)   == "up");
}

// ---------------------------------------------------------------------------
// map_gestures no-op stub compiles and runs without recording
// ---------------------------------------------------------------------------

TEST_CASE("expander mock map_gestures is a no-op",
          "[mock][expander]") {
    internal::basic_expander e;
    expander_mock h;

    h.map_gestures(e);

    CHECK(h.calls().empty());
}
