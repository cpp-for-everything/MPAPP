// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::table_view`.

#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/table_view_handler.hpp>
#include <mpapp/handlers/mock/text_cell_handler.hpp>
#include <mpapp/handlers/mock/switch_cell_handler.hpp>
#include <mpapp/switch_cell.hpp>
#include <mpapp/table_view.hpp>
#include <mpapp/text_cell.hpp>

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

TEST_CASE("typed_sections starts empty + records count via mock handler",
          "[mock][table_view][typed]") {
    table_view tv;
    table_view_handler<platform::mock> h;
    h.map_typed_sections(tv);
    h.clear_calls();

    CHECK(tv.typed_sections.get().empty());

    text_cell   name{};
    text_cell   email{};
    switch_cell notifications{};

    tv.typed_sections = std::vector<table_section_typed>{
        table_section_typed{
            "Profile",
            std::vector<cell*>{ &name, &email }
        },
        table_section_typed{
            "Preferences",
            std::vector<cell*>{ &notifications }
        },
    };

    REQUIRE(tv.typed_sections.get().size()                 == 2);
    CHECK  (tv.typed_sections.get()[0].title               == "Profile");
    REQUIRE(tv.typed_sections.get()[0].rows.size()         == 2);
    CHECK  (tv.typed_sections.get()[0].rows[0]             == &name);
    CHECK  (tv.typed_sections.get()[0].rows[1]             == &email);
    CHECK  (tv.typed_sections.get()[1].title               == "Preferences");
    REQUIRE(tv.typed_sections.get()[1].rows.size()         == 1);
    CHECK  (tv.typed_sections.get()[1].rows[0]             == &notifications);

    REQUIRE(h.calls_as_strings().size() == 1);
    CHECK  (h.calls_as_strings()[0]     == "typed_sections.count=2");
}

TEST_CASE("typed_sections + flat sections can coexist on the surface",
          "[mock][table_view][typed]") {
    // The handler picks typed_sections when non-empty; otherwise flat.
    // The surface itself doesn't enforce mutual exclusion — both
    // Observables hold values independently. Verify both can be set
    // and read back.
    table_view tv;
    text_cell  row_a{};

    tv.add_section("Flat header");
    tv.add_row(0, "Flat row");

    tv.typed_sections = std::vector<table_section_typed>{
        table_section_typed{ "Typed header", std::vector<cell*>{ &row_a } }
    };

    CHECK(tv.sections.get().size()                  == 1);
    CHECK(tv.sections.get()[0].rows.size()          == 1);
    CHECK(tv.typed_sections.get().size()            == 1);
    CHECK(tv.typed_sections.get()[0].rows.size()    == 1);
}
