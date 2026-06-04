// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for mpapp::semantics attached properties
// and mpapp::mock_semantic_screen_reader.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/accessibility/semantics.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// Minimal concrete view for tests (view is non-copyable, non-movable).
// ---------------------------------------------------------------------------
namespace {

class test_view : public view {
public:
    test_view() = default;
};

} // namespace

// ---------------------------------------------------------------------------
// heading_level to_string
// ---------------------------------------------------------------------------

TEST_CASE("heading_level to_string covers every case and fallback",
          "[mock][semantics]") {
    // Arrange + Act + Assert (each case)
    CHECK(to_string(heading_level::none)   == "none");
    CHECK(to_string(heading_level::level1) == "level1");
    CHECK(to_string(heading_level::level2) == "level2");
    CHECK(to_string(heading_level::level3) == "level3");
    CHECK(to_string(heading_level::level4) == "level4");
    CHECK(to_string(heading_level::level5) == "level5");
    CHECK(to_string(heading_level::level6) == "level6");
    // out-of-range fallback
    CHECK(to_string(static_cast<heading_level>(99)) == "?");
}

// ---------------------------------------------------------------------------
// semantics::description
// ---------------------------------------------------------------------------

TEST_CASE("semantics get_description returns empty string by default",
          "[mock][semantics]") {
    test_view v;
    // Arrange: fresh view not yet in the store
    // Act + Assert
    CHECK(semantics::get_description(v) == "");
    semantics::clear(v);
}

TEST_CASE("semantics set_description and get_description round-trip",
          "[mock][semantics]") {
    test_view v;
    // Arrange
    semantics::set_description(v, "Profile photo");
    // Act
    const auto result = semantics::get_description(v);
    // Assert
    CHECK(result == "Profile photo");
    semantics::clear(v);
}

TEST_CASE("semantics set_description overwrites previous value",
          "[mock][semantics]") {
    test_view v;
    semantics::set_description(v, "First");
    semantics::set_description(v, "Second");
    CHECK(semantics::get_description(v) == "Second");
    semantics::clear(v);
}

TEST_CASE("semantics description is independent per view instance",
          "[mock][semantics]") {
    test_view a;
    test_view b;
    semantics::set_description(a, "View A");
    semantics::set_description(b, "View B");
    CHECK(semantics::get_description(a) == "View A");
    CHECK(semantics::get_description(b) == "View B");
    semantics::clear(a);
    semantics::clear(b);
}

// ---------------------------------------------------------------------------
// semantics::hint
// ---------------------------------------------------------------------------

TEST_CASE("semantics get_hint returns empty string by default",
          "[mock][semantics]") {
    test_view v;
    CHECK(semantics::get_hint(v) == "");
    semantics::clear(v);
}

TEST_CASE("semantics set_hint and get_hint round-trip",
          "[mock][semantics]") {
    test_view v;
    semantics::set_hint(v, "Double-tap to activate");
    CHECK(semantics::get_hint(v) == "Double-tap to activate");
    semantics::clear(v);
}

TEST_CASE("semantics set_hint overwrites previous value",
          "[mock][semantics]") {
    test_view v;
    semantics::set_hint(v, "Old hint");
    semantics::set_hint(v, "New hint");
    CHECK(semantics::get_hint(v) == "New hint");
    semantics::clear(v);
}

TEST_CASE("semantics hint is independent per view instance",
          "[mock][semantics]") {
    test_view a;
    test_view b;
    semantics::set_hint(a, "Hint A");
    semantics::set_hint(b, "Hint B");
    CHECK(semantics::get_hint(a) == "Hint A");
    CHECK(semantics::get_hint(b) == "Hint B");
    semantics::clear(a);
    semantics::clear(b);
}

// ---------------------------------------------------------------------------
// semantics::heading_level
// ---------------------------------------------------------------------------

TEST_CASE("semantics get_heading_level returns none by default",
          "[mock][semantics]") {
    test_view v;
    CHECK(semantics::get_heading_level(v) == heading_level::none);
    semantics::clear(v);
}

TEST_CASE("semantics set_heading_level and get_heading_level round-trip",
          "[mock][semantics]") {
    test_view v;
    semantics::set_heading_level(v, heading_level::level2);
    CHECK(semantics::get_heading_level(v) == heading_level::level2);
    semantics::clear(v);
}

TEST_CASE("semantics heading_level covers all enum values",
          "[mock][semantics]") {
    test_view v;
    for (auto lvl : { heading_level::none, heading_level::level1,
                      heading_level::level2, heading_level::level3,
                      heading_level::level4, heading_level::level5,
                      heading_level::level6 }) {
        semantics::set_heading_level(v, lvl);
        CHECK(semantics::get_heading_level(v) == lvl);
    }
    semantics::clear(v);
}

TEST_CASE("semantics set_heading_level overwrites previous value",
          "[mock][semantics]") {
    test_view v;
    semantics::set_heading_level(v, heading_level::level1);
    semantics::set_heading_level(v, heading_level::level4);
    CHECK(semantics::get_heading_level(v) == heading_level::level4);
    semantics::clear(v);
}

TEST_CASE("semantics heading_level is independent per view instance",
          "[mock][semantics]") {
    test_view a;
    test_view b;
    semantics::set_heading_level(a, heading_level::level1);
    semantics::set_heading_level(b, heading_level::level3);
    CHECK(semantics::get_heading_level(a) == heading_level::level1);
    CHECK(semantics::get_heading_level(b) == heading_level::level3);
    semantics::clear(a);
    semantics::clear(b);
}

// ---------------------------------------------------------------------------
// semantics::is_in_accessible_tree
// ---------------------------------------------------------------------------

TEST_CASE("semantics get_is_in_accessible_tree returns true by default",
          "[mock][semantics]") {
    test_view v;
    CHECK(semantics::get_is_in_accessible_tree(v) == true);
    semantics::clear(v);
}

TEST_CASE("semantics set_is_in_accessible_tree false and read back",
          "[mock][semantics]") {
    test_view v;
    semantics::set_is_in_accessible_tree(v, false);
    CHECK(semantics::get_is_in_accessible_tree(v) == false);
    semantics::clear(v);
}

TEST_CASE("semantics set_is_in_accessible_tree true after false",
          "[mock][semantics]") {
    test_view v;
    semantics::set_is_in_accessible_tree(v, false);
    semantics::set_is_in_accessible_tree(v, true);
    CHECK(semantics::get_is_in_accessible_tree(v) == true);
    semantics::clear(v);
}

TEST_CASE("semantics is_in_accessible_tree is independent per view instance",
          "[mock][semantics]") {
    test_view a;
    test_view b;
    semantics::set_is_in_accessible_tree(a, false);
    semantics::set_is_in_accessible_tree(b, true);
    CHECK(semantics::get_is_in_accessible_tree(a) == false);
    CHECK(semantics::get_is_in_accessible_tree(b) == true);
    semantics::clear(a);
    semantics::clear(b);
}

// ---------------------------------------------------------------------------
// semantics::clear
// ---------------------------------------------------------------------------

TEST_CASE("semantics clear removes all properties for that view",
          "[mock][semantics]") {
    test_view v;
    semantics::set_description(v, "Desc");
    semantics::set_hint(v, "Hint");
    semantics::set_heading_level(v, heading_level::level3);
    semantics::set_is_in_accessible_tree(v, false);

    semantics::clear(v);

    // All should be back to defaults
    CHECK(semantics::get_description(v)          == "");
    CHECK(semantics::get_hint(v)                 == "");
    CHECK(semantics::get_heading_level(v)        == heading_level::none);
    CHECK(semantics::get_is_in_accessible_tree(v) == true);

    semantics::clear(v); // idempotent: clearing an absent entry is safe
}

TEST_CASE("semantics clear is idempotent on a view with no properties",
          "[mock][semantics]") {
    test_view v;
    // Should not throw or crash
    semantics::clear(v);
    semantics::clear(v);
    CHECK(semantics::get_description(v) == "");
}

// ---------------------------------------------------------------------------
// Multiple properties on the same view co-exist
// ---------------------------------------------------------------------------

TEST_CASE("semantics multiple properties on same view are independent",
          "[mock][semantics]") {
    test_view v;
    semantics::set_description(v, "My view");
    semantics::set_hint(v, "Swipe left");
    semantics::set_heading_level(v, heading_level::level1);
    semantics::set_is_in_accessible_tree(v, false);

    CHECK(semantics::get_description(v)           == "My view");
    CHECK(semantics::get_hint(v)                  == "Swipe left");
    CHECK(semantics::get_heading_level(v)         == heading_level::level1);
    CHECK(semantics::get_is_in_accessible_tree(v) == false);

    // Mutate one; others stay
    semantics::set_description(v, "Updated");
    CHECK(semantics::get_description(v)           == "Updated");
    CHECK(semantics::get_hint(v)                  == "Swipe left");
    CHECK(semantics::get_heading_level(v)         == heading_level::level1);
    CHECK(semantics::get_is_in_accessible_tree(v) == false);

    semantics::clear(v);
}

// ---------------------------------------------------------------------------
// mock_semantic_screen_reader
// ---------------------------------------------------------------------------

TEST_CASE("mock_semantic_screen_reader starts with no announcements",
          "[mock][semantics]") {
    mock_semantic_screen_reader reader;
    CHECK(reader.announcements().empty());
    CHECK(reader.empty());
}

TEST_CASE("mock_semantic_screen_reader records a single announcement",
          "[mock][semantics]") {
    mock_semantic_screen_reader reader;
    reader.announce("Welcome to the app");
    REQUIRE(reader.announcements().size() == 1);
    CHECK(reader.announcements()[0] == "Welcome to the app");
    CHECK(!reader.empty());
}

TEST_CASE("mock_semantic_screen_reader records multiple announcements in order",
          "[mock][semantics]") {
    mock_semantic_screen_reader reader;
    reader.announce("First");
    reader.announce("Second");
    reader.announce("Third");

    const auto& log = reader.announcements();
    REQUIRE(log.size() == 3);
    CHECK(log[0] == "First");
    CHECK(log[1] == "Second");
    CHECK(log[2] == "Third");
}

TEST_CASE("mock_semantic_screen_reader clear_announcements resets log",
          "[mock][semantics]") {
    mock_semantic_screen_reader reader;
    reader.announce("Hello");
    reader.clear_announcements();
    CHECK(reader.announcements().empty());
    CHECK(reader.empty());
}

TEST_CASE("mock_semantic_screen_reader announce via base interface pointer",
          "[mock][semantics]") {
    // Arrange: hold via abstract base
    mock_semantic_screen_reader concrete;
    semantic_screen_reader& base = concrete;

    // Act
    base.announce("Announced through base");

    // Assert: recorded in the mock
    REQUIRE(concrete.announcements().size() == 1);
    CHECK(concrete.announcements()[0] == "Announced through base");
}

TEST_CASE("mock_semantic_screen_reader announce accepts empty string",
          "[mock][semantics]") {
    mock_semantic_screen_reader reader;
    reader.announce("");
    REQUIRE(reader.announcements().size() == 1);
    CHECK(reader.announcements()[0] == "");
}

TEST_CASE("mock_semantic_screen_reader clear then re-announce works",
          "[mock][semantics]") {
    mock_semantic_screen_reader reader;
    reader.announce("A");
    reader.clear_announcements();
    reader.announce("B");
    REQUIRE(reader.announcements().size() == 1);
    CHECK(reader.announcements()[0] == "B");
}
