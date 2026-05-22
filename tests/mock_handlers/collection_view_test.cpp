// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::collection_view`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/collection_view.hpp>
#include <mpapp/handlers/mock/collection_view_handler.hpp>
#include <mpapp/label.hpp>
#include <mpapp/text_cell.hpp>

using namespace mpapp;

TEST_CASE("collection_view defaults are sensible",
          "[mock][collection_view]") {
    collection_view cv;
    CHECK(cv.items_source.get().empty());
    CHECK(cv.selection_mode.get()   == collection_selection_mode::single);
    CHECK(cv.selected_index.get()   == -1);
    CHECK(cv.selected_indices.get().empty());
    CHECK(cv.layout.get()           == collection_layout::vertical_list);
}

TEST_CASE("select() honors selection_mode = none",
          "[mock][collection_view]") {
    collection_view cv;
    cv.items_source = std::vector<std::string>{"x", "y"};
    cv.selection_mode = collection_selection_mode::none;

    cv.select(1);
    CHECK(cv.selected_index.get()  == -1);
    CHECK(cv.selected_indices.get().empty());
}

TEST_CASE("select() in single mode sets one entry",
          "[mock][collection_view]") {
    collection_view cv;
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
    collection_view cv;
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
    collection_view cv;
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
    collection_view cv;
    cv.items_source = std::vector<std::string>{"x", "y"};
    cv.select(1);
    cv.clear_selection();
    CHECK(cv.selected_index.get() == -1);
    CHECK(cv.selected_indices.get().empty());
}

TEST_CASE("typed_items starts empty",
          "[mock][collection_view][typed]") {
    collection_view cv;
    CHECK(cv.typed_items.get().empty());
}

TEST_CASE("typed_items holds non-owning view* pointers",
          "[mock][collection_view][typed]") {
    collection_view cv;
    label       a{};
    label       b{};
    text_cell   c{};

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
    collection_view cv;
    label           a{};

    cv.items_source = std::vector<std::string>{"flat-1", "flat-2"};
    cv.typed_items  = std::vector<view*>{ &a };

    CHECK(cv.items_source.get().size() == 2);
    CHECK(cv.typed_items.get().size()  == 1);
    CHECK(cv.typed_items.get()[0]      == &a);
}
