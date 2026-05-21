// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::graphics_view`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/graphics_view.hpp>
#include <mpapp/handlers/mock/graphics_view_handler.hpp>

using namespace mpapp;

TEST_CASE("graphics_view defaults",
          "[mock][graphics_view]") {
    graphics_view gv;
    CHECK(gv.width.get()  == 0);
    CHECK(gv.height.get() == 0);
    CHECK(gv.draw_count.get() == 0);
}

TEST_CASE("invalidate() bumps draw_count and emits draw_requested",
          "[mock][graphics_view]") {
    graphics_view gv;
    int hits = 0;
    struct cb_t { int* hits; void operator()() const { ++*hits; } };
    cb_t cb{&hits};
    signal_slot<> slot{};
    gv.draw_requested.subscribe(slot, cb);

    gv.invalidate();
    gv.invalidate();
    gv.invalidate();

    CHECK(gv.draw_count.get() == 3);
    CHECK(hits == 3);
}

TEST_CASE("mock handler records draw_count + size",
          "[mock][graphics_view]") {
    graphics_view gv;
    graphics_view_handler<platform::mock> h;
    h.map_draw_count(gv);
    h.map_size(gv);
    h.clear_calls();

    gv.width  = 800;
    gv.height = 600;
    gv.invalidate();

    REQUIRE(h.calls_as_strings().size() == 3);
    CHECK(h.calls_as_strings()[0] == "width=800");
    CHECK(h.calls_as_strings()[1] == "height=600");
    CHECK(h.calls_as_strings()[2] == "draw_count=1");
}
