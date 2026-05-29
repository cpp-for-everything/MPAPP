// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for basic_carousel_view (the CarouselView gap).

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/carousel_view_handler.hpp>
#include <mpapp/internal/basic_carousel_view.hpp>

using namespace mpapp;

namespace {
using carousel_mock = internal::carousel_view_handler<platform::mock>;
} // namespace

TEST_CASE("carousel_view defaults", "[mock][carousel_view]") {
    internal::basic_carousel_view c;
    CHECK(c.position.get() == 0);
    CHECK(c.loop.get() == true);
    CHECK(c.is_swipe_enabled.get() == true);
    CHECK(c.peek_count.get() == 0);
    CHECK(c.item_count() == 0);
}

TEST_CASE("carousel_view scroll_to clamps when loop is off",
          "[mock][carousel_view]") {
    internal::basic_carousel_view c;
    c.items_source = std::vector<std::string>{ "a", "b", "c" };
    c.loop = false;

    int last = -1;
    int fires = 0;
    signal_slot<int> slot;
    auto cb = [&](int p) { last = p; ++fires; };
    c.position_changed.subscribe(slot, cb);

    c.scroll_to(2);
    CHECK(c.position.get() == 2);
    CHECK(fires == 1);
    CHECK(last == 2);

    c.scroll_to(99);            // clamp to last (2) -> already there -> no fire
    CHECK(c.position.get() == 2);
    CHECK(fires == 1);

    c.scroll_to(-5);            // clamp to 0
    CHECK(c.position.get() == 0);
    CHECK(fires == 2);
}

TEST_CASE("carousel_view scroll_to wraps when loop is on",
          "[mock][carousel_view]") {
    internal::basic_carousel_view c;
    c.items_source = std::vector<std::string>{ "a", "b", "c" };
    c.loop = true;

    c.scroll_to(3);             // wraps to 0 -> already at 0 -> no change
    CHECK(c.position.get() == 0);

    c.scroll_to(4);             // wraps to 1
    CHECK(c.position.get() == 1);

    c.scroll_to(-1);            // wraps to 2
    CHECK(c.position.get() == 2);
}

TEST_CASE("carousel_view mock handler records mappers + swipe",
          "[mock][carousel_view]") {
    internal::basic_carousel_view c;
    carousel_mock                 h;
    c.items_source = std::vector<std::string>{ "x", "y" };

    h.map_items_source(c);
    h.map_position(c);
    h.map_loop(c);
    h.clear_calls();

    c.position = 1;             // bound -> recorded
    h.simulate_swipe(c, 0);     // scroll_to(0) -> position 0 + position_changed

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "position=1",
        "position=0",            // scroll_to set position back to 0
        "position_changed=0",
    });
}
