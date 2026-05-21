// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::table_view`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/table_view_handler.hpp>
#include <mpapp/table_view.hpp>

using namespace mpapp;

TEST_CASE("table_view starts empty",
          "[mock][table_view]") {
    table_view tv;
    CHECK(tv.sections.get().empty());
    CHECK(tv.intent.get()     == table_intent::data);
    CHECK(tv.row_height.get() == -1);
    CHECK(tv.total_row_count() == 0);
}

TEST_CASE("add_section + add_row updates sections + total_row_count",
          "[mock][table_view]") {
    table_view tv;
    tv.add_section("Profile");
    tv.add_section("Preferences");

    tv.add_row(0, "Name");
    tv.add_row(0, "Email");
    tv.add_row(1, "Notifications");

    REQUIRE(tv.sections.get().size() == 2);
    CHECK(tv.sections.get()[0].title         == "Profile");
    CHECK(tv.sections.get()[0].rows.size()   == 2);
    CHECK(tv.sections.get()[1].title         == "Preferences");
    CHECK(tv.sections.get()[1].rows.size()   == 1);
    CHECK(tv.total_row_count() == 3);
}

TEST_CASE("add_row to invalid section is a no-op",
          "[mock][table_view]") {
    table_view tv;
    tv.add_section("Only one");
    tv.add_row(5, "ignored");
    CHECK(tv.sections.get()[0].rows.empty());
    CHECK(tv.total_row_count() == 0);
}

TEST_CASE("mock handler records sections.count + row_height",
          "[mock][table_view]") {
    table_view tv;
    table_view_handler<platform::mock> h;
    h.map_sections(tv);
    h.map_row_height(tv);
    h.clear_calls();

    tv.add_section("First");
    tv.add_section("Second");
    tv.row_height = 44;

    REQUIRE(h.calls_as_strings().size() == 3);
    CHECK(h.calls_as_strings()[0] == "sections.count=1");
    CHECK(h.calls_as_strings()[1] == "sections.count=2");
    CHECK(h.calls_as_strings()[2] == "row_height=44");
}
