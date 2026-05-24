// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_web_view`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/web_view_handler.hpp>
#include <mpapp/web_view.hpp>

using namespace mpapp;

TEST_CASE("web_view defaults",
          "[mock][web_view]") {
    internal::basic_web_view wv;
    CHECK(wv.url.get().empty());
    CHECK(wv.html_source.get().empty());
    CHECK(wv.is_loading.get()     == false);
    CHECK(wv.can_go_back.get()    == false);
    CHECK(wv.can_go_forward.get() == false);
}

TEST_CASE("load() updates url + emits navigating/navigated",
          "[mock][web_view]") {
    internal::basic_web_view wv;
    int nav_starts = 0, nav_ends = 0;
    std::string seen;
    struct s_cb { int* n; std::string* seen;
                  void operator()(const std::string& v) const { ++*n; *seen = v; } };
    struct e_cb { int* n;
                  void operator()(const std::string&, bool) const { ++*n; } };
    s_cb start{&nav_starts, &seen};
    e_cb finish{&nav_ends};
    signal_slot<const std::string&> ss{};
    signal_slot<const std::string&, bool> es{};
    wv.navigating.subscribe(ss, start);
    wv.navigated.subscribe(es, finish);

    wv.load("https://example.com");
    CHECK(wv.url.get()        == "https://example.com");
    CHECK(wv.is_loading.get() == false);   // mock completes synchronously
    CHECK(nav_starts == 1);
    CHECK(nav_ends   == 1);
    CHECK(seen       == "https://example.com");
}

TEST_CASE("mock handler records url + is_loading",
          "[mock][web_view]") {
    internal::basic_web_view wv;
    web_view_handler<platform::mock> h;
    h.map_url(wv);
    h.map_is_loading(wv);
    h.clear_calls();

    wv.load("https://test.local");
    // load() sets url, then is_loading=true, then is_loading=false
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "url=https://test.local",
        "is_loading=true",
        "is_loading=false",
    });
}
