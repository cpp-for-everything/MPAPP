// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::navigation_page` and the page_stack engine.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/navigation_page_handler.hpp>
#include <mpapp/navigation_page.hpp>
#include <mpapp/page.hpp>

using namespace mpapp;

TEST_CASE("navigation_page starts empty when default-constructed",
          "[mock][navigation_page]") {
    navigation_page nav;
    CHECK(nav.stack().depth()  == 0);
    CHECK(nav.stack().top()    == nullptr);
    CHECK(nav.stack().root()   == nullptr);
    CHECK(nav.current_page.get() == nullptr);
    CHECK(nav.root_page.get()    == nullptr);
    CHECK(nav.stack_depth.get()  == 0);
}

TEST_CASE("navigation_page sets root from ctor argument",
          "[mock][navigation_page]") {
    page home;
    navigation_page nav(&home);
    CHECK(nav.stack().depth()    == 1);
    CHECK(nav.stack().top()      == static_cast<view*>(&home));
    CHECK(nav.stack().root()     == static_cast<view*>(&home));
    CHECK(nav.current_page.get() == &home);
    CHECK(nav.root_page.get()    == &home);
    CHECK(nav.stack_depth.get()  == 1);
}

TEST_CASE("push / pop adjusts stack and current_page",
          "[mock][navigation_page]") {
    page home;
    page details;
    navigation_page nav(&home);

    nav.push(&details);
    CHECK(nav.stack_depth.get()  == 2);
    CHECK(nav.current_page.get() == &details);
    CHECK(nav.root_page.get()    == &home);

    page* popped = nav.pop();
    CHECK(popped == &details);
    CHECK(nav.stack_depth.get()  == 1);
    CHECK(nav.current_page.get() == &home);
}

TEST_CASE("pop_to_root collapses all intermediate pages",
          "[mock][navigation_page]") {
    page a, b, c, d;
    navigation_page nav(&a);
    nav.push(&b);
    nav.push(&c);
    nav.push(&d);
    CHECK(nav.stack_depth.get() == 4);

    nav.pop_to_root();
    CHECK(nav.stack_depth.get()  == 1);
    CHECK(nav.current_page.get() == &a);
}

TEST_CASE("attached property store keys on child page",
          "[mock][navigation_page]") {
    page home, settings;
    navigation_page nav(&home);
    nav.push(&settings);

    // Default for both pages: has_back_button=true, has_navigation_bar=true.
    CHECK(nav.get_has_back_button(home)        == true);
    CHECK(nav.get_has_back_button(settings)    == true);
    CHECK(nav.get_has_navigation_bar(home)     == true);
    CHECK(nav.get_back_button_title(home)      == std::string{});

    // Set on the leaf; doesn't affect the root.
    nav.set_has_back_button(settings, false);
    nav.set_back_button_title(settings, "Done");
    CHECK(nav.get_has_back_button(settings)    == false);
    CHECK(nav.get_back_button_title(settings)  == "Done");
    CHECK(nav.get_has_back_button(home)        == true);  // unchanged
}

TEST_CASE("mock handler records lifecycle signals around push/pop",
          "[mock][navigation_page]") {
    page home, details;
    navigation_page nav(&home);
    navigation_page_handler<platform::mock> h;

    h.map_stack(nav);
    h.clear_calls();   // discard the bind-time stack.depth record

    nav.push(&details);
    // Expect: will_disappear(home), will_appear(details),
    //         did_disappear(home), did_appear(details).
    auto rendered = h.calls_as_strings();
    REQUIRE(rendered.size() == 4);
    CHECK(rendered[0] == "page_will_disappear=true");
    CHECK(rendered[1] == "page_will_appear=true");
    CHECK(rendered[2] == "page_did_disappear=true");
    CHECK(rendered[3] == "page_did_appear=true");
}

TEST_CASE("insert_page_before places a page below the top",
          "[mock][navigation_page]") {
    page a, b, inserted;
    navigation_page nav(&a);
    nav.push(&b);
    nav.insert_page_before(&b, &inserted);

    CHECK(nav.stack_depth.get()  == 3);
    CHECK(nav.current_page.get() == &b);     // top unchanged
    CHECK(nav.root_page.get()    == &a);     // root unchanged
    // Stack: [a, inserted, b]
    const auto& pages = nav.stack().pages();
    REQUIRE(pages.size() == 3);
    CHECK(pages[0] == &a);
    CHECK(pages[1] == &inserted);
    CHECK(pages[2] == &b);
}

TEST_CASE("remove_page removes top as pop and middle without changing top",
          "[mock][navigation_page]") {
    page a, b, c;
    navigation_page nav(&a);
    nav.push(&b);
    nav.push(&c);

    // Remove a non-top page: top stays the same.
    nav.remove_page(&b);
    CHECK(nav.stack_depth.get()  == 2);
    CHECK(nav.current_page.get() == &c);

    // Remove the top: becomes a pop.
    nav.remove_page(&c);
    CHECK(nav.stack_depth.get()  == 1);
    CHECK(nav.current_page.get() == &a);
}
