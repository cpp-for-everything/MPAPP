// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_collection_view`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_collection_view.hpp>
#include <mpapp/handlers/mock/collection_view_handler.hpp>
#include <mpapp/internal/basic_label.hpp>
#include <mpapp/internal/basic_text_cell.hpp>

using namespace mpapp;

TEST_CASE("collection_view defaults are sensible",
          "[mock][collection_view]") {
    internal::basic_collection_view cv;
    CHECK(cv.items_source.get().empty());
    CHECK(cv.selection_mode.get()   == collection_selection_mode::single);
    CHECK(cv.selected_index.get()   == -1);
    CHECK(cv.selected_indices.get().empty());
    CHECK(cv.layout.get()           == collection_layout::vertical_list);
}

TEST_CASE("select() honors selection_mode = none",
          "[mock][collection_view]") {
    internal::basic_collection_view cv;
    cv.items_source = std::vector<std::string>{"x", "y"};
    cv.selection_mode = collection_selection_mode::none;

    cv.select(1);
    CHECK(cv.selected_index.get()  == -1);
    CHECK(cv.selected_indices.get().empty());
}

TEST_CASE("select() in single mode sets one entry",
          "[mock][collection_view]") {
    internal::basic_collection_view cv;
    cv.items_source = std::vector<std::string>{"x", "y", "z"};

    cv.select(2);
    CHECK(cv.selected_index.get() == 2);
    REQUIRE(cv.selected_indices.get().size() == 1);
    CHECK(cv.selected_indices.get()[0] == 2);

    cv.select(0);
    CHECK(cv.selected_index.get() == 0);
    REQUIRE(cv.selected_indices.get().size() == 1);
    CHECK(cv.selected_indices.get()[0] == 0);
}

TEST_CASE("select() in multiple mode appends",
          "[mock][collection_view]") {
    internal::basic_collection_view cv;
    cv.items_source = std::vector<std::string>{"x", "y", "z", "w"};
    cv.selection_mode = collection_selection_mode::multiple;

    cv.select(1);
    cv.select(3);
    cv.select(1);   // already selected -> no duplicate

    REQUIRE(cv.selected_indices.get().size() == 2);
    CHECK(cv.selected_indices.get()[0] == 1);
    CHECK(cv.selected_indices.get()[1] == 3);
}

TEST_CASE("deselect() removes from multi-select",
          "[mock][collection_view]") {
    internal::basic_collection_view cv;
    cv.items_source = std::vector<std::string>{"x", "y", "z"};
    cv.selection_mode = collection_selection_mode::multiple;
    cv.select(0);
    cv.select(1);
    cv.select(2);

    cv.deselect(1);
    REQUIRE(cv.selected_indices.get().size() == 2);
    CHECK(cv.selected_indices.get()[0] == 0);
    CHECK(cv.selected_indices.get()[1] == 2);
}

TEST_CASE("clear_selection wipes both surfaces",
          "[mock][collection_view]") {
    internal::basic_collection_view cv;
    cv.items_source = std::vector<std::string>{"x", "y"};
    cv.select(1);
    cv.clear_selection();
    CHECK(cv.selected_index.get() == -1);
    CHECK(cv.selected_indices.get().empty());
}

TEST_CASE("typed_items starts empty",
          "[mock][collection_view][typed]") {
    internal::basic_collection_view cv;
    CHECK(cv.typed_items.get().empty());
}

TEST_CASE("typed_items holds non-owning view* pointers",
          "[mock][collection_view][typed]") {
    internal::basic_collection_view cv;
    internal::basic_label a{};
    internal::basic_label b{};
    internal::basic_text_cell   c{};

    cv.typed_items = std::vector<view*>{ &a, &b, &c };

    REQUIRE(cv.typed_items.get().size() == 3);
    CHECK(cv.typed_items.get()[0] == &a);
    CHECK(cv.typed_items.get()[1] == &b);
    CHECK(cv.typed_items.get()[2] == &c);
}

TEST_CASE("typed_items + items_source can coexist on the surface",
          "[mock][collection_view][typed]") {
    // Handler picks typed when non-empty; otherwise flat. Surface
    // doesn't enforce mutual exclusion.
    internal::basic_collection_view cv;
    internal::basic_label a{};

    cv.items_source = std::vector<std::string>{"flat-1", "flat-2"};
    cv.typed_items  = std::vector<view*>{ &a };

    CHECK(cv.items_source.get().size() == 2);
    CHECK(cv.typed_items.get().size()  == 1);
    CHECK(cv.typed_items.get()[0]      == &a);
}

// ---- item_template ---------------------------------------------------------

TEST_CASE("item_template materializes a cell per items_source row",
          "[mock][collection_view][template]") {
    internal::basic_collection_view cv;

    cv.item_template = [](int i) -> std::unique_ptr<view> {
        auto c  = std::make_unique<internal::basic_text_cell>();
        c->text = "row-" + std::to_string(i);
        return c;
    };
    cv.items_source = std::vector<std::string>{"a", "b", "c", "d"};

    REQUIRE(cv.materialized_count() == 4);
    auto vs = cv.materialized_views();
    REQUIRE(vs.size() == 4);
    // We constructed text_cell so each pointer should be non-null and
    // downcast-able. Check via static_cast (we know the concrete type).
    for (int i = 0; i < 4; ++i) {
        auto* tc = static_cast<internal::basic_text_cell*>(vs[static_cast<std::size_t>(i)]);
        REQUIRE(tc != nullptr);
        CHECK(tc->text.get() == "row-" + std::to_string(i));
    }
}

TEST_CASE("item_template re-materializes when items_source changes",
          "[mock][collection_view][template]") {
    internal::basic_collection_view cv;
    cv.item_template = [](int) -> std::unique_ptr<view> {
        return std::make_unique<internal::basic_text_cell>();
    };

    cv.items_source = std::vector<std::string>{"a", "b"};
    CHECK(cv.materialized_count() == 2);

    cv.items_source = std::vector<std::string>{"x", "y", "z", "w", "u"};
    CHECK(cv.materialized_count() == 5);

    cv.items_source = std::vector<std::string>{};
    CHECK(cv.materialized_count() == 0);
}

TEST_CASE("item_template re-materializes when template changes",
          "[mock][collection_view][template]") {
    internal::basic_collection_view cv;
    cv.items_source = std::vector<std::string>{"a", "b", "c"};

    // No template set yet — materialized is empty.
    CHECK(cv.materialized_count() == 0);

    cv.item_template = [](int) { return std::make_unique<internal::basic_text_cell>(); };
    CHECK(cv.materialized_count() == 3);

    // Replace with a different template — re-materialize.
    cv.item_template = [](int) { return std::make_unique<internal::basic_label>(); };
    CHECK(cv.materialized_count() == 3);

    // Clear the template — materialized clears.
    cv.item_template = internal::basic_collection_view::item_factory_t{};
    CHECK(cv.materialized_count() == 0);
}

TEST_CASE("item_template factory receives the row index",
          "[mock][collection_view][template]") {
    internal::basic_collection_view  cv;
    std::vector<int> seen;

    // Capture by value won't survive — capture by reference into a
    // member-like external so the lambda is short-lived but the
    // observed-during-materialize behavior is recorded.
    cv.item_template = [&seen](int i) -> std::unique_ptr<view> {
        seen.push_back(i);
        return std::make_unique<internal::basic_text_cell>();
    };
    cv.items_source = std::vector<std::string>{"r0", "r1", "r2"};

    REQUIRE(seen.size() == 3);
    CHECK(seen[0] == 0);
    CHECK(seen[1] == 1);
    CHECK(seen[2] == 2);
}

TEST_CASE("item_template doesn't override typed_items on the surface",
          "[mock][collection_view][template]") {
    // The surface holds both; the handler decides precedence. Verify
    // they remain independent on the C++ side.
    internal::basic_collection_view cv;
    internal::basic_label a{};
    cv.typed_items  = std::vector<view*>{ &a };
    cv.item_template = [](int) { return std::make_unique<internal::basic_text_cell>(); };
    cv.items_source = std::vector<std::string>{"a", "b"};

    CHECK(cv.typed_items.get().size() == 1);
    CHECK(cv.materialized_count()     == 2);
}

TEST_CASE("materialized_changed fires on rematerialize",
          "[mock][collection_view][template]") {
    internal::basic_collection_view cv;
    int hits = 0;
    struct cb_t {
        int* hits;
        void operator()() const { ++*hits; }
    };
    cb_t cb{&hits};
    signal_slot<> slot{};
    cv.materialized_changed.subscribe(slot, cb);

    // No template + items change → still fires (with empty materialize).
    cv.items_source = std::vector<std::string>{"a"};
    CHECK(hits == 1);

    // Template set → fires.
    cv.item_template = [](int) { return std::make_unique<internal::basic_text_cell>(); };
    CHECK(hits == 2);

    // items_source change → fires.
    cv.items_source = std::vector<std::string>{"x", "y"};
    CHECK(hits == 3);

    // Template cleared → fires (materialize empties).
    cv.item_template = internal::basic_collection_view::item_factory_t{};
    CHECK(hits == 4);
}

TEST_CASE("layout default is vertical_list and map_layout records it",
          "[mock][collection_view][layout]") {
    internal::basic_collection_view cv;
    internal::collection_view_handler<platform::mock> h;
    h.map_layout(cv);

    CHECK(cv.layout.get() == collection_layout::vertical_list);
    CHECK(h.last_layout    == collection_layout::vertical_list);
}

TEST_CASE("layout cycles through all four enum values",
          "[mock][collection_view][layout]") {
    internal::basic_collection_view cv;
    internal::collection_view_handler<platform::mock> h;
    h.map_layout(cv);

    cv.layout = collection_layout::horizontal_list;
    CHECK(h.last_layout == collection_layout::horizontal_list);

    cv.layout = collection_layout::vertical_grid;
    CHECK(h.last_layout == collection_layout::vertical_grid);

    cv.layout = collection_layout::horizontal_grid;
    CHECK(h.last_layout == collection_layout::horizontal_grid);

    cv.layout = collection_layout::vertical_list;
    CHECK(h.last_layout == collection_layout::vertical_list);
}

TEST_CASE("layout change does not perturb items_source or selection",
          "[mock][collection_view][layout]") {
    internal::basic_collection_view cv;
    cv.items_source   = std::vector<std::string>{"a", "b", "c"};
    cv.selection_mode = collection_selection_mode::multiple;
    cv.select(0);
    cv.select(2);

    // Cycle layout — items + selection survive untouched.
    for (auto l : {collection_layout::horizontal_list,
                   collection_layout::vertical_grid,
                   collection_layout::horizontal_grid,
                   collection_layout::vertical_list}) {
        cv.layout = l;
        REQUIRE(cv.items_source.get().size() == 3);
        CHECK(cv.items_source.get()[1] == "b");
        REQUIRE(cv.selected_indices.get().size() == 2);
        CHECK(cv.selected_indices.get()[0] == 0);
        CHECK(cv.selected_indices.get()[1] == 2);
    }
}
