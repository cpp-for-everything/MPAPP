// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::shell`.

#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/shell_handler.hpp>
#include <mpapp/shell.hpp>

using namespace mpapp;

TEST_CASE("shell defaults",
          "[mock][shell]") {
    shell s;
    CHECK(s.current_route.get()       == "//");
    CHECK(s.current_tab_index.get()   == 0);
    CHECK(s.is_flyout_open.get()      == false);
    CHECK(s.tabs.get().empty());
    CHECK(s.registered_routes.get().empty());
}

TEST_CASE("register_route deduplicates",
          "[mock][shell]") {
    shell s;
    s.register_route("home");
    s.register_route("settings");
    s.register_route("home");     // duplicate, ignored
    REQUIRE(s.registered_routes.get().size() == 2);
    CHECK(s.registered_routes.get()[0] == "home");
    CHECK(s.registered_routes.get()[1] == "settings");
}

TEST_CASE("go_to(//tab_name) updates current_tab_index by label match",
          "[mock][shell]") {
    shell s;
    s.add_tab("home");
    s.add_tab("library");
    s.add_tab("settings");

    s.go_to("//library");
    CHECK(s.current_route.get()     == "//library");
    CHECK(s.current_tab_index.get() == 1);

    s.go_to("//settings/details/42");
    CHECK(s.current_route.get()     == "//settings/details/42");
    CHECK(s.current_tab_index.get() == 2);   // leaf segment ignored for tab match

    s.go_to("//does_not_exist");
    CHECK(s.current_route.get()     == "//does_not_exist");
    CHECK(s.current_tab_index.get() == 2);   // unchanged on unknown tab
}

TEST_CASE("navigated signal fires after each go_to",
          "[mock][shell]") {
    shell s;
    int hits = 0;
    std::string last;
    struct cb_t { int* hits; std::string* last;
                  void operator()(const std::string& v) const { ++*hits; *last = v; } };
    cb_t cb{&hits, &last};
    signal_slot<const std::string&> slot{};
    s.navigated.subscribe(slot, cb);

    s.go_to("//home");
    s.go_to("//settings");
    CHECK(hits == 2);
    CHECK(last == "//settings");
}

TEST_CASE("flyout helpers + flyout_toggled signal",
          "[mock][shell]") {
    shell s;
    int hits = 0;
    bool last = false;
    struct cb_t { int* hits; bool* last;
                  void operator()(bool v) const { ++*hits; *last = v; } };
    cb_t cb{&hits, &last};
    signal_slot<bool> slot{};
    s.flyout_toggled.subscribe(slot, cb);

    s.open_flyout();
    CHECK(s.is_flyout_open.get() == true);
    CHECK(hits == 1);
    CHECK(last == true);

    s.open_flyout();             // no flip
    CHECK(hits == 1);

    s.toggle_flyout();
    CHECK(s.is_flyout_open.get() == false);
    CHECK(hits == 2);
    CHECK(last == false);
}

TEST_CASE("mock handler records route + tab + flyout changes",
          "[mock][shell]") {
    shell s;
    s.add_tab("home");
    s.add_tab("library");

    shell_handler<platform::mock> h;
    h.map_current_route(s);
    h.map_current_tab_index(s);
    h.map_is_flyout_open(s);
    h.clear_calls();

    s.go_to("//library");
    s.open_flyout();

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "current_tab_index=1",
        "current_route=//library",
        "is_flyout_open=true",
    });
}

TEST_CASE("shell.current_content tracks page swaps",
          "[mock][shell]") {
    page home, library;
    shell s;

    CHECK(s.current_content.get() == nullptr);

    s.current_content = &home;
    CHECK(s.current_content.get() == &home);

    s.current_content = &library;
    CHECK(s.current_content.get() == &library);

    s.current_content = nullptr;
    CHECK(s.current_content.get() == nullptr);
}
