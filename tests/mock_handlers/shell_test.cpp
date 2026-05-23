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

TEST_CASE("can_activate guard blocks navigation when false",
          "[mock][shell][guard]") {
    shell s;
    s.add_tab("home");
    s.add_tab("settings");
    s.go_to("//home");

    bool guard_called = false;
    std::string seen_target;
    s.can_activate = [&](std::string_view target) {
        guard_called = true;
        seen_target  = std::string{target};
        return false;
    };

    int blocked_hits = 0;
    std::string blocked_target;
    struct cb_t {
        int*         hits;
        std::string* target;
        void operator()(const std::string& v) const { ++*hits; *target = v; }
    };
    cb_t cb{&blocked_hits, &blocked_target};
    signal_slot<const std::string&> slot{};
    s.navigation_blocked.subscribe(slot, cb);

    s.go_to("//settings");

    CHECK(guard_called);
    CHECK(seen_target           == "//settings");
    CHECK(blocked_hits          == 1);
    CHECK(blocked_target        == "//settings");
    // State unchanged.
    CHECK(s.current_route.get()     == "//home");
    CHECK(s.current_tab_index.get() == 0);
}

TEST_CASE("can_activate returning true lets navigation proceed",
          "[mock][shell][guard]") {
    shell s;
    s.add_tab("home");
    s.add_tab("settings");

    int guard_calls = 0;
    s.can_activate = [&](std::string_view) {
        ++guard_calls;
        return true;
    };

    s.go_to("//settings");
    CHECK(guard_calls               == 1);
    CHECK(s.current_route.get()     == "//settings");
    CHECK(s.current_tab_index.get() == 1);
}

TEST_CASE("can_activate guard applies to typed go_to too",
          "[mock][shell][guard][route]") {
    // The typed go_to<Path, &Table>(args...) delegates to the
    // string-based go_to, so a single guard covers both.
    struct dummy_page : page {};
    static constexpr auto routes = route_table{
        route<"home",        dummy_page>{},
        route<"home/details", dummy_page,
              param<"id", int>>{},
    };

    shell s;
    s.add_tab("home");
    s.can_activate = [](std::string_view target) {
        // Block anything with a query string.
        return target.find('?') == std::string_view::npos;
    };

    s.go_to<"home", &routes>();
    CHECK(s.current_route.get() == "//home");

    s.go_to<"home/details", &routes>(42);
    // Blocked by guard.
    CHECK(s.current_route.get() == "//home");
}

TEST_CASE("can_deactivate guard receives current + target",
          "[mock][shell][guard]") {
    shell s;
    s.add_tab("home");
    s.add_tab("settings");
    s.go_to("//home");

    std::string seen_current;
    std::string seen_target;
    s.can_deactivate = [&](std::string_view current, std::string_view target) {
        seen_current = std::string{current};
        seen_target  = std::string{target};
        return true;
    };

    s.go_to("//settings");
    CHECK(seen_current           == "//home");
    CHECK(seen_target            == "//settings");
    CHECK(s.current_route.get()  == "//settings");
}

TEST_CASE("can_deactivate returning false blocks navigation",
          "[mock][shell][guard]") {
    shell s;
    s.add_tab("home");
    s.add_tab("settings");
    s.go_to("//home");

    int blocked_hits = 0;
    struct cb_t {
        int* hits;
        void operator()(const std::string&) const { ++*hits; }
    };
    cb_t cb{&blocked_hits};
    signal_slot<const std::string&> slot{};
    s.navigation_blocked.subscribe(slot, cb);

    s.can_deactivate = [](std::string_view, std::string_view) { return false; };
    s.go_to("//settings");

    CHECK(blocked_hits          == 1);
    CHECK(s.current_route.get() == "//home");
}

TEST_CASE("deactivate fires before activate guard",
          "[mock][shell][guard]") {
    shell s;
    s.add_tab("home");
    s.add_tab("settings");
    s.go_to("//home");

    std::vector<std::string> order;
    s.can_deactivate = [&](std::string_view, std::string_view) {
        order.emplace_back("deactivate");
        return true;
    };
    s.can_activate = [&](std::string_view) {
        order.emplace_back("activate");
        return true;
    };

    s.go_to("//settings");
    REQUIRE(order.size() == 2);
    CHECK(order[0] == "deactivate");
    CHECK(order[1] == "activate");
}

TEST_CASE("activate is skipped if deactivate rejects",
          "[mock][shell][guard]") {
    shell s;
    s.add_tab("home");
    s.add_tab("settings");
    s.go_to("//home");

    bool activate_called = false;
    s.can_deactivate = [](std::string_view, std::string_view) { return false; };
    s.can_activate   = [&](std::string_view) {
        activate_called = true;
        return true;
    };

    s.go_to("//settings");
    CHECK_FALSE(activate_called);
    CHECK(s.current_route.get() == "//home");
}

TEST_CASE("navigated_to fires on the current page after go_to",
          "[mock][shell][lifecycle]") {
    page p;
    shell s;
    s.current_content = &p;

    std::vector<std::string> seen;
    struct cb_t {
        std::vector<std::string>* dst;
        void operator()(const std::string& v) const { dst->push_back(v); }
    };
    cb_t cb{&seen};
    signal_slot<const std::string&> slot{};
    p.navigated_to.subscribe(slot, cb);

    s.go_to("//home");
    s.go_to("//details/42");
    REQUIRE(seen.size() == 2);
    CHECK(seen[0] == "//home");
    CHECK(seen[1] == "//details/42");
}

TEST_CASE("navigated_from fires on the outgoing page with previous URI",
          "[mock][shell][lifecycle]") {
    page p;
    shell s;
    s.current_content = &p;
    s.go_to("//home");  // p is current; sees navigated_to("//home")

    std::vector<std::string> from_seen;
    struct cb_t {
        std::vector<std::string>* dst;
        void operator()(const std::string& v) const { dst->push_back(v); }
    };
    cb_t cb{&from_seen};
    signal_slot<const std::string&> slot{};
    p.navigated_from.subscribe(slot, cb);

    s.go_to("//settings");
    // navigated_from fires BEFORE current_route updates, so the URI
    // is the previous one ("//home"), not "//settings".
    REQUIRE(from_seen.size() == 1);
    CHECK(from_seen[0] == "//home");
}

TEST_CASE("navigated_to/from do NOT fire when guards block",
          "[mock][shell][lifecycle]") {
    page p;
    shell s;
    s.current_content = &p;
    s.go_to("//home");  // initial enter

    int to_hits = 0, from_hits = 0;
    struct cb_t {
        int* hits;
        void operator()(const std::string&) const { ++*hits; }
    };
    cb_t cb_to{&to_hits};
    cb_t cb_from{&from_hits};
    signal_slot<const std::string&> slot_to{};
    signal_slot<const std::string&> slot_from{};
    p.navigated_to.subscribe(slot_to, cb_to);
    p.navigated_from.subscribe(slot_from, cb_from);

    s.can_activate = [](std::string_view) { return false; };
    s.go_to("//settings");

    CHECK(to_hits   == 0);
    CHECK(from_hits == 0);
}
