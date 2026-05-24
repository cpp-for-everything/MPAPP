// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_table_view`.

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
    internal::basic_table_view tv;
    CHECK(tv.sections.get().empty());
    CHECK(tv.intent.get()     == table_intent::data);
    CHECK(tv.row_height.get() == -1);
    CHECK(tv.total_row_count() == 0);
}

TEST_CASE("add_section + add_row updates sections + total_row_count",
          "[mock][table_view]") {
    internal::basic_table_view tv;
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
    internal::basic_table_view tv;
    tv.add_section("Only one");
    tv.add_row(5, "ignored");
    CHECK(tv.sections.get()[0].rows.empty());
    CHECK(tv.total_row_count() == 0);
}

TEST_CASE("mock handler records sections.count + row_height",
          "[mock][table_view]") {
    internal::basic_table_view tv;
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
    internal::basic_table_view tv;
    table_view_handler<platform::mock> h;
    h.map_typed_sections(tv);
    h.clear_calls();

    CHECK(tv.typed_sections.get().empty());

    internal::basic_text_cell   name{};
    internal::basic_text_cell   email{};
    internal::basic_switch_cell notifications{};

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
    internal::basic_table_view tv;
    internal::basic_text_cell  row_a{};

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

TEST_CASE("cell_at resolves typed-section coordinates",
          "[mock][table_view][typed]") {
    internal::basic_table_view  tv;
    internal::basic_text_cell   a{};
    internal::basic_text_cell   b{};
    internal::basic_switch_cell c{};

    tv.typed_sections = std::vector<table_section_typed>{
        table_section_typed{ "Profile", std::vector<cell*>{ &a, &b } },
        table_section_typed{ "Prefs",   std::vector<cell*>{ &c }     },
    };

    CHECK(tv.cell_at(0, 0) == &a);
    CHECK(tv.cell_at(0, 1) == &b);
    CHECK(tv.cell_at(1, 0) == &c);

    // Out-of-range coordinates return nullptr.
    CHECK(tv.cell_at(0, 2)  == nullptr);   // row past end
    CHECK(tv.cell_at(2, 0)  == nullptr);   // section past end
    CHECK(tv.cell_at(-1, 0) == nullptr);   // negative section
    CHECK(tv.cell_at(0, -1) == nullptr);   // negative row
}

TEST_CASE("cell_at returns nullptr when typed_sections is empty",
          "[mock][table_view][typed]") {
    // Flat sections only — cell_at can't resolve any coordinate.
    internal::basic_table_view tv;
    tv.add_section("Flat");
    tv.add_row(0, "Row 0");
    tv.add_row(0, "Row 1");

    CHECK(tv.cell_at(0, 0) == nullptr);
    CHECK(tv.cell_at(0, 1) == nullptr);
}

TEST_CASE("cell.tapped emits via cell_at lookup as handlers do at runtime",
          "[mock][table_view][typed]") {
    // Simulates what handlers do after row_tapped: look up the cell at
    // (section, row) via cell_at() and emit its tapped signal.
    internal::basic_table_view tv;
    internal::basic_text_cell  row{};

    tv.typed_sections = std::vector<table_section_typed>{
        table_section_typed{ "Settings", std::vector<cell*>{ &row } }
    };

    int hits = 0;
    struct cb_t { int* hits; void operator()() const { ++*hits; } };
    cb_t cb{&hits};
    signal_slot<> slot{};
    row.tapped.subscribe(slot, cb);

    // Handler-side dispatch: a tap on (0, 0) resolves the cell and
    // fires its tapped signal.
    if (cell* c = tv.cell_at(0, 0); c != nullptr) {
        c->tapped.emit();
    }
    CHECK(hits == 1);

    // A tap on an invalid coordinate is a no-op.
    if (cell* c = tv.cell_at(5, 0); c != nullptr) {
        c->tapped.emit();
    }
    CHECK(hits == 1);
}
