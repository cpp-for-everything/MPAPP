// SPDX-License-Identifier: Apache-2.0
// Tests for mpapp::route_table — the compile-time route table per
// ADR-0016. Mostly compile-time assertions (anything that compiles
// is a passing test); a small number of runtime checks cover the
// URI-building + runtime_has lookup paths.

#include <string>
#include <tuple>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_page.hpp>
#include <mpapp/route.hpp>
#include <mpapp/internal/basic_shell.hpp>

using namespace mpapp;

namespace {

// Stand-in page subclasses — instances are never created in tests but
// the types flow through the route_table for compile-time lookups.
struct home_page    : internal::basic_page {};
struct details_page : internal::basic_page {};
struct settings_page: internal::basic_page {};

inline constexpr auto test_routes = route_table{
    route<"home",         home_page>{},
    route<"home/details", details_page,
          param<"id", int>>{},
    route<"settings",     settings_page>{},
    route<"profile",      home_page,
          param<"who", std::string>,
          param<"verbose", bool>>{},
};

using table_t = std::remove_const_t<decltype(test_routes)>;

} // namespace

// ---- Compile-time has<> --------------------------------------------------

static_assert( table_t::has<"home">());
static_assert( table_t::has<"home/details">());
static_assert( table_t::has<"settings">());
static_assert( table_t::has<"profile">());
static_assert(!table_t::has<"nope">());
static_assert(!table_t::has<"home/missing">());

// ---- Compile-time route_for / params_for ---------------------------------

static_assert(std::is_same_v<table_t::route_for<"home">::page_t,         home_page>);
static_assert(std::is_same_v<table_t::route_for<"home/details">::page_t, details_page>);
static_assert(std::is_same_v<table_t::page_for<"settings">,              settings_page>);

static_assert(std::tuple_size_v<table_t::params_for<"home">>          == 0);
static_assert(std::tuple_size_v<table_t::params_for<"home/details">>  == 1);
static_assert(std::tuple_size_v<table_t::params_for<"profile">>       == 2);

static_assert(std::is_same_v<
    std::tuple_element_t<0, table_t::params_for<"home/details">>::value_t,
    int>);
static_assert(std::is_same_v<
    std::tuple_element_t<0, table_t::params_for<"profile">>::value_t,
    std::string>);
static_assert(std::is_same_v<
    std::tuple_element_t<1, table_t::params_for<"profile">>::value_t,
    bool>);

// ---- size + count --------------------------------------------------------

static_assert(table_t::size == 4);

// ---- Runtime lookup ------------------------------------------------------

TEST_CASE("route_table::runtime_has matches by string",
          "[mock][shell][route]") {
    CHECK( table_t::runtime_has("home"));
    CHECK( table_t::runtime_has("home/details"));
    CHECK( table_t::runtime_has("settings"));
    CHECK( table_t::runtime_has("profile"));
    CHECK(!table_t::runtime_has("nope"));
    CHECK(!table_t::runtime_has(""));
}

// ---- URI building --------------------------------------------------------

TEST_CASE("build_uri assembles paths with no params",
          "[mock][shell][route]") {
    CHECK(table_t::build_uri<"home">()     == "//home");
    CHECK(table_t::build_uri<"settings">() == "//settings");
}

TEST_CASE("build_uri assembles paths with one int param",
          "[mock][shell][route]") {
    CHECK(table_t::build_uri<"home/details">(42) == "//home/details?id=42");
}

TEST_CASE("build_uri assembles paths with multiple typed params",
          "[mock][shell][route]") {
    CHECK(table_t::build_uri<"profile">(std::string{"ada"}, true)
          == "//profile?who=ada&verbose=true");
    CHECK(table_t::build_uri<"profile">(std::string{"bob"}, false)
          == "//profile?who=bob&verbose=false");
}

// ---- Shell::go_to<Path, &Table>() integration ----------------------------

TEST_CASE("shell::go_to<Path, &Table>(args...) routes through string go_to",
          "[mock][shell][route]") {
    internal::basic_shell s;
    s.add_tab("home");
    s.add_tab("settings");
    s.add_tab("profile");

    // Compile-time-checked navigation: typo here would fail static_assert.
    s.go_to<"home/details", &test_routes>(42);
    CHECK(s.current_route.get()     == "//home/details?id=42");
    CHECK(s.current_tab_index.get() == 0);  // tab "home" matched

    s.go_to<"settings", &test_routes>();
    CHECK(s.current_route.get()     == "//settings");
    CHECK(s.current_tab_index.get() == 1);

    s.go_to<"profile", &test_routes>(std::string{"ada"}, true);
    CHECK(s.current_route.get()     == "//profile?who=ada&verbose=true");
    CHECK(s.current_tab_index.get() == 2);
}

TEST_CASE("shell::go_to<Path, &Table>() fires navigated signal",
          "[mock][shell][route]") {
    internal::basic_shell s;
    int hits = 0;
    std::string last;
    struct cb_t {
        int*         hits;
        std::string* last;
        void operator()(const std::string& v) const { ++*hits; *last = v; }
    };
    cb_t cb{&hits, &last};
    signal_slot<const std::string&> slot{};
    s.navigated.subscribe(slot, cb);

    s.go_to<"home", &test_routes>();
    s.go_to<"home/details", &test_routes>(7);
    CHECK(hits == 2);
    CHECK(last == "//home/details?id=7");
}
