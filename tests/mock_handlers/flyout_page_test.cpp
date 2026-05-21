// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::flyout_page`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/flyout_page.hpp>
#include <mpapp/handlers/mock/flyout_page_handler.hpp>
#include <mpapp/page.hpp>

using namespace mpapp;

TEST_CASE("flyout_page defaults",
          "[mock][flyout_page]") {
    flyout_page fp;
    CHECK(fp.flyout.get()       == nullptr);
    CHECK(fp.detail.get()       == nullptr);
    CHECK(fp.is_presented.get() == false);
    CHECK(fp.layout_behavior.get() == flyout_layout_behavior::default_);
}

TEST_CASE("present / dismiss / toggle drive is_presented",
          "[mock][flyout_page]") {
    flyout_page fp;
    fp.present();
    CHECK(fp.is_presented.get() == true);

    fp.dismiss();
    CHECK(fp.is_presented.get() == false);

    fp.toggle();
    CHECK(fp.is_presented.get() == true);
    fp.toggle();
    CHECK(fp.is_presented.get() == false);
}

TEST_CASE("presented_changed signal fires on flip only",
          "[mock][flyout_page]") {
    flyout_page fp;
    int hits = 0;
    bool last = false;
    struct cb_t {
        int* hits; bool* last;
        void operator()(bool v) const { ++*hits; *last = v; }
    };
    cb_t cb{&hits, &last};
    signal_slot<bool> slot{};
    fp.presented_changed.subscribe(slot, cb);

    fp.present();
    CHECK(hits == 1);
    CHECK(last == true);
    fp.present();   // no flip
    CHECK(hits == 1);
    fp.dismiss();
    CHECK(hits == 2);
    CHECK(last == false);
}

TEST_CASE("mock handler records flyout + detail + is_presented",
          "[mock][flyout_page]") {
    page menu, content;
    flyout_page fp;
    flyout_page_handler<platform::mock> h;

    h.map_flyout(fp);
    h.map_detail(fp);
    h.map_is_presented(fp);
    h.clear_calls();

    fp.flyout = &menu;
    fp.detail = &content;
    fp.present();
    fp.dismiss();

    auto rendered = h.calls_as_strings();
    REQUIRE(rendered.size() == 4);
    CHECK(rendered[0] == "flyout.present=true");
    CHECK(rendered[1] == "detail.present=true");
    CHECK(rendered[2] == "is_presented=true");
    CHECK(rendered[3] == "is_presented=false");
}
