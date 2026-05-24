// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_tabbed_page`.

#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/tabbed_page_handler.hpp>
#include <mpapp/page.hpp>
#include <mpapp/tabbed_page.hpp>

using namespace mpapp;

TEST_CASE("tabbed_page starts empty",
          "[mock][tabbed_page]") {
    internal::basic_tabbed_page tp;
    CHECK(tp.children.get().empty());
    CHECK(tp.selected_index.get() == 0);
    CHECK(tp.current_page.get()   == nullptr);
}

TEST_CASE("add_tab appends and selects first by default",
          "[mock][tabbed_page]") {
    internal::basic_page a, b;
    internal::basic_tabbed_page tp;

    tp.add_tab(&a);
    CHECK(tp.children.get().size() == 1);
    CHECK(tp.selected_index.get()  == 0);
    CHECK(tp.current_page.get()    == &a);

    tp.add_tab(&b);
    CHECK(tp.children.get().size() == 2);
    CHECK(tp.selected_index.get()  == 0);
    CHECK(tp.current_page.get()    == &a);   // selection unchanged
}

TEST_CASE("select() clamps and updates current_page",
          "[mock][tabbed_page]") {
    internal::basic_page a, b, c;
    internal::basic_tabbed_page tp;
    tp.add_tab(&a);
    tp.add_tab(&b);
    tp.add_tab(&c);

    tp.select(2);
    CHECK(tp.selected_index.get() == 2);
    CHECK(tp.current_page.get()   == &c);

    tp.select(99);                 // clamps to 2 (last)
    CHECK(tp.selected_index.get() == 2);

    tp.select(-5);                 // clamps to 0
    CHECK(tp.selected_index.get() == 0);
    CHECK(tp.current_page.get()   == &a);
}

TEST_CASE("remove_tab clamps the selection",
          "[mock][tabbed_page]") {
    internal::basic_page a, b, c;
    internal::basic_tabbed_page tp;
    tp.add_tab(&a);
    tp.add_tab(&b);
    tp.add_tab(&c);
    tp.select(2);

    tp.remove_tab(&c);
    CHECK(tp.children.get().size() == 2);
    CHECK(tp.selected_index.get()  == 1);   // clamped from 2
    CHECK(tp.current_page.get()    == &b);
}

TEST_CASE("mock handler records lifecycle on tab switch",
          "[mock][tabbed_page]") {
    internal::basic_page a, b;
    internal::basic_tabbed_page tp;
    tp.add_tab(&a);
    tp.add_tab(&b);

    tabbed_page_handler<platform::mock> h;
    h.map_lifecycle(tp);
    h.clear_calls();

    tp.select(1);
    auto rendered = h.calls_as_strings();
    // Order: tab_will_disappear(a), tab_will_appear(b),
    //        tab_did_disappear(a), tab_did_appear(b).
    REQUIRE(rendered.size() == 4);
    CHECK(rendered[0] == "tab_will_disappear=true");
    CHECK(rendered[1] == "tab_will_appear=true");
    CHECK(rendered[2] == "tab_did_disappear=true");
    CHECK(rendered[3] == "tab_did_appear=true");
}
