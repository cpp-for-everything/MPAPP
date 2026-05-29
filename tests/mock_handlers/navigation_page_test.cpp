// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_navigation_page` and the page_stack engine.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/executor.hpp>
#include <mpapp/handlers/mock/navigation_page_handler.hpp>
#include <mpapp/internal/basic_navigation_page.hpp>
#include <mpapp/internal/basic_page.hpp>
#include <mpapp/test_dispatcher.hpp>

using namespace mpapp;

TEST_CASE("navigation_page starts empty when default-constructed",
          "[mock][navigation_page]") {
    internal::basic_navigation_page nav;
    CHECK(nav.stack().depth()  == 0);
    CHECK(nav.stack().top()    == nullptr);
    CHECK(nav.stack().root()   == nullptr);
    CHECK(nav.current_page.get() == nullptr);
    CHECK(nav.root_page.get()    == nullptr);
    CHECK(nav.stack_depth.get()  == 0);
}

TEST_CASE("navigation_page sets root from ctor argument",
          "[mock][navigation_page]") {
    internal::basic_page home;
    internal::basic_navigation_page nav(&home);
    CHECK(nav.stack().depth()    == 1);
    CHECK(nav.stack().top()      == static_cast<view*>(&home));
    CHECK(nav.stack().root()     == static_cast<view*>(&home));
    CHECK(nav.current_page.get() == &home);
    CHECK(nav.root_page.get()    == &home);
    CHECK(nav.stack_depth.get()  == 1);
}

TEST_CASE("push / pop adjusts stack and current_page",
          "[mock][navigation_page]") {
    internal::basic_page home;
    internal::basic_page details;
    internal::basic_navigation_page nav(&home);

    nav.push(&details);
    CHECK(nav.stack_depth.get()  == 2);
    CHECK(nav.current_page.get() == &details);
    CHECK(nav.root_page.get()    == &home);

    internal::basic_page* popped = nav.pop();
    CHECK(popped == &details);
    CHECK(nav.stack_depth.get()  == 1);
    CHECK(nav.current_page.get() == &home);
}

TEST_CASE("pop_to_root collapses all intermediate pages",
          "[mock][navigation_page]") {
    internal::basic_page a, b, c, d;
    internal::basic_navigation_page nav(&a);
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
    internal::basic_page home, settings;
    internal::basic_navigation_page nav(&home);
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
    internal::basic_page home, details;
    internal::basic_navigation_page nav(&home);
    internal::navigation_page_handler<platform::mock> h;

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
    internal::basic_page a, b, inserted;
    internal::basic_navigation_page nav(&a);
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
    internal::basic_page a, b, c;
    internal::basic_navigation_page nav(&a);
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

// --- Async wrappers (ADR-0014 + ADR-0019) -----------------------------------

TEST_CASE("push_async completes synchronously in the mock build",
          "[mock][navigation_page][async]") {
    internal::basic_page home, details;
    internal::basic_navigation_page nav(&home);

    auto t = nav.push_async(&details);
    // The coroutine body just calls push() — no suspension point —
    // so the task is ready immediately after the eager-start.
    REQUIRE(t.is_ready());
    t.await_resume();   // void: no value to extract

    CHECK(nav.stack_depth.get()  == 2);
    CHECK(nav.current_page.get() == &details);
}

TEST_CASE("pop_async returns the popped page",
          "[mock][navigation_page][async]") {
    internal::basic_page home, details;
    internal::basic_navigation_page nav(&home);
    nav.push(&details);

    auto t = nav.pop_async();
    REQUIRE(t.is_ready());
    internal::basic_page* popped = t.await_resume();

    CHECK(popped == &details);
    CHECK(nav.stack_depth.get()  == 1);
    CHECK(nav.current_page.get() == &home);
}

TEST_CASE("pop_to_root_async collapses the stack",
          "[mock][navigation_page][async]") {
    internal::basic_page a, b, c, d;
    internal::basic_navigation_page nav(&a);
    nav.push(&b);
    nav.push(&c);
    nav.push(&d);
    REQUIRE(nav.stack_depth.get() == 4);

    auto t = nav.pop_to_root_async();
    REQUIRE(t.is_ready());
    t.await_resume();

    CHECK(nav.stack_depth.get()  == 1);
    CHECK(nav.current_page.get() == &a);
}

TEST_CASE("async wrappers compose under co_await",
          "[mock][navigation_page][async]") {
    internal::basic_page a, b, c;
    internal::basic_navigation_page nav(&a);

    auto scenario = [](internal::basic_navigation_page& n,
                       internal::basic_page* x,
                       internal::basic_page* y) -> task<int> {
        co_await n.push_async(x);
        co_await n.push_async(y);
        auto* popped = co_await n.pop_async();
        co_return (popped == y) ? 1 : 0;
    };

    auto t = scenario(nav, &b, &c);
    REQUIRE(t.is_ready());
    CHECK(t.await_resume()       == 1);
    CHECK(nav.stack_depth.get()  == 2);
    CHECK(nav.current_page.get() == &b);
}
