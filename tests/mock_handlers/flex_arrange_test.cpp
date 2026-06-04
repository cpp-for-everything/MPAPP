// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Unit tests for the pure `mpapp::flex_arrange` solver.

#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <mpapp/layout/flex_arrange.hpp>

using namespace mpapp;
using Catch::Matchers::WithinAbs;

namespace {

constexpr double kTol = 1e-9;

void check_rect(const flex_rect& got, const flex_rect& want) {
    CHECK_THAT(got.x,      WithinAbs(want.x, kTol));
    CHECK_THAT(got.y,      WithinAbs(want.y, kTol));
    CHECK_THAT(got.width,  WithinAbs(want.width, kTol));
    CHECK_THAT(got.height, WithinAbs(want.height, kTol));
}

// Convenience builder: fixed basis item with given grow/shrink and cross.
flex_item_input item(double basis, double grow, double shrink,
                     double cross) {
    flex_item_input it;
    it.basis          = basis;
    it.grow           = grow;
    it.shrink         = shrink;
    it.measured_cross = cross;
    it.measured_main  = basis < 0.0 ? 0.0 : basis;
    return it;
}

} // namespace

TEST_CASE("single line row distributes free space by grow",
          "[flex][grow]") {
    // Container 300 wide. Two items basis 50, grow 1 and 2.
    // Free = 300 - 100 = 200. Item0 gets 200*1/3, item1 gets 200*2/3.
    flex_container_input c;
    c.width  = 300.0;
    c.height = 100.0;
    c.align_items = flex_align_items::start;

    std::vector<flex_item_input> items{
        item(50.0, 1.0, 1.0, 40.0),
        item(50.0, 2.0, 1.0, 40.0),
    };

    const auto rects = flex_arrange(c, items);
    REQUIRE(rects.size() == 2);

    const double w0 = 50.0 + 200.0 * (1.0 / 3.0);
    const double w1 = 50.0 + 200.0 * (2.0 / 3.0);
    check_rect(rects[0], {0.0, 0.0, w0, 40.0});
    check_rect(rects[1], {w0, 0.0, w1, 40.0});
}

TEST_CASE("no grow leaves items at basis with free space at end",
          "[flex][grow]") {
    flex_container_input c;
    c.width  = 300.0;
    c.height = 100.0;
    c.align_items = flex_align_items::start;

    std::vector<flex_item_input> items{
        item(60.0, 0.0, 1.0, 30.0),
        item(60.0, 0.0, 1.0, 30.0),
    };

    const auto rects = flex_arrange(c, items);
    check_rect(rects[0], {0.0, 0.0, 60.0, 30.0});
    check_rect(rects[1], {60.0, 0.0, 60.0, 30.0});
}

TEST_CASE("over capacity row shrinks weighted by shrink times basis",
          "[flex][shrink]") {
    // Container 100 wide, two items basis 80 each (total 160).
    // Deficit = 60. Equal shrink*basis -> each shrinks 30 -> 50 each.
    flex_container_input c;
    c.width  = 100.0;
    c.height = 50.0;
    c.wrap   = flex_wrap::no_wrap;
    c.align_items = flex_align_items::start;

    std::vector<flex_item_input> items{
        item(80.0, 0.0, 1.0, 20.0),
        item(80.0, 0.0, 1.0, 20.0),
    };

    const auto rects = flex_arrange(c, items);
    check_rect(rects[0], {0.0, 0.0, 50.0, 20.0});
    check_rect(rects[1], {50.0, 0.0, 50.0, 20.0});
}

TEST_CASE("wrap breaks items into multiple lines",
          "[flex][wrap]") {
    // Container 100 wide. Three items basis 60. First fits, second
    // overflows -> new line, third overflows again -> third line.
    flex_container_input c;
    c.width  = 100.0;
    c.height = 300.0;
    c.wrap   = flex_wrap::wrap;
    c.align_items   = flex_align_items::start;
    c.align_content = flex_align_content::start;

    std::vector<flex_item_input> items{
        item(60.0, 0.0, 0.0, 30.0),
        item(60.0, 0.0, 0.0, 30.0),
        item(60.0, 0.0, 0.0, 30.0),
    };

    const auto rects = flex_arrange(c, items);
    // Each on its own line, stacked on the cross (y) axis at line heights.
    check_rect(rects[0], {0.0, 0.0,  60.0, 30.0});
    check_rect(rects[1], {0.0, 30.0, 60.0, 30.0});
    check_rect(rects[2], {0.0, 60.0, 60.0, 30.0});
}

TEST_CASE("two items per line wrap with gap",
          "[flex][wrap]") {
    // Container 100. Items basis 40, main_gap 10. Two fit (40+10+40=90),
    // third wraps.
    flex_container_input c;
    c.width    = 100.0;
    c.height   = 300.0;
    c.wrap     = flex_wrap::wrap;
    c.main_gap = 10.0;
    c.cross_gap = 5.0;
    c.align_items   = flex_align_items::start;
    c.align_content = flex_align_content::start;

    std::vector<flex_item_input> items{
        item(40.0, 0.0, 0.0, 20.0),
        item(40.0, 0.0, 0.0, 20.0),
        item(40.0, 0.0, 0.0, 20.0),
    };

    const auto rects = flex_arrange(c, items);
    check_rect(rects[0], {0.0,  0.0,  40.0, 20.0});
    check_rect(rects[1], {50.0, 0.0,  40.0, 20.0});
    // Third on second line: cross offset = 20 (line0) + 5 (gap).
    check_rect(rects[2], {0.0,  25.0, 40.0, 20.0});
}

TEST_CASE("justify_content start places items at the front",
          "[flex][justify]") {
    flex_container_input c;
    c.width  = 200.0;
    c.height = 50.0;
    c.justify_content = flex_justify::start;
    c.align_items = flex_align_items::start;

    std::vector<flex_item_input> items{
        item(40.0, 0.0, 0.0, 10.0),
        item(40.0, 0.0, 0.0, 10.0),
    };

    const auto rects = flex_arrange(c, items);
    check_rect(rects[0], {0.0,  0.0, 40.0, 10.0});
    check_rect(rects[1], {40.0, 0.0, 40.0, 10.0});
}

TEST_CASE("justify_content center centers the group",
          "[flex][justify]") {
    // Used = 80, free = 120, offset = 60.
    flex_container_input c;
    c.width  = 200.0;
    c.height = 50.0;
    c.justify_content = flex_justify::center;
    c.align_items = flex_align_items::start;

    std::vector<flex_item_input> items{
        item(40.0, 0.0, 0.0, 10.0),
        item(40.0, 0.0, 0.0, 10.0),
    };

    const auto rects = flex_arrange(c, items);
    check_rect(rects[0], {60.0,  0.0, 40.0, 10.0});
    check_rect(rects[1], {100.0, 0.0, 40.0, 10.0});
}

TEST_CASE("justify_content end pushes items to the back",
          "[flex][justify]") {
    // Free = 120, offset = 120.
    flex_container_input c;
    c.width  = 200.0;
    c.height = 50.0;
    c.justify_content = flex_justify::end;
    c.align_items = flex_align_items::start;

    std::vector<flex_item_input> items{
        item(40.0, 0.0, 0.0, 10.0),
        item(40.0, 0.0, 0.0, 10.0),
    };

    const auto rects = flex_arrange(c, items);
    check_rect(rects[0], {120.0, 0.0, 40.0, 10.0});
    check_rect(rects[1], {160.0, 0.0, 40.0, 10.0});
}

TEST_CASE("justify_content space_between hugs the edges",
          "[flex][justify]") {
    // Free = 120 split into 1 gap of 120 between two items.
    flex_container_input c;
    c.width  = 200.0;
    c.height = 50.0;
    c.justify_content = flex_justify::space_between;
    c.align_items = flex_align_items::start;

    std::vector<flex_item_input> items{
        item(40.0, 0.0, 0.0, 10.0),
        item(40.0, 0.0, 0.0, 10.0),
    };

    const auto rects = flex_arrange(c, items);
    check_rect(rects[0], {0.0,   0.0, 40.0, 10.0});
    check_rect(rects[1], {160.0, 0.0, 40.0, 10.0});
}

TEST_CASE("justify_content space_around adds half-units at the edges",
          "[flex][justify]") {
    // Free = 120, unit = 60. offset = 30, between = 60.
    flex_container_input c;
    c.width  = 200.0;
    c.height = 50.0;
    c.justify_content = flex_justify::space_around;
    c.align_items = flex_align_items::start;

    std::vector<flex_item_input> items{
        item(40.0, 0.0, 0.0, 10.0),
        item(40.0, 0.0, 0.0, 10.0),
    };

    const auto rects = flex_arrange(c, items);
    // item0 at 30; item1 at 30 + 40 + 60 = 130.
    check_rect(rects[0], {30.0,  0.0, 40.0, 10.0});
    check_rect(rects[1], {130.0, 0.0, 40.0, 10.0});
}

TEST_CASE("justify_content space_evenly uses equal gaps everywhere",
          "[flex][justify]") {
    // Free = 120, unit = 120/3 = 40. offset = 40, between = 40.
    flex_container_input c;
    c.width  = 200.0;
    c.height = 50.0;
    c.justify_content = flex_justify::space_evenly;
    c.align_items = flex_align_items::start;

    std::vector<flex_item_input> items{
        item(40.0, 0.0, 0.0, 10.0),
        item(40.0, 0.0, 0.0, 10.0),
    };

    const auto rects = flex_arrange(c, items);
    // item0 at 40; item1 at 40 + 40 + 40 = 120.
    check_rect(rects[0], {40.0,  0.0, 40.0, 10.0});
    check_rect(rects[1], {120.0, 0.0, 40.0, 10.0});
}

TEST_CASE("align_items stretch fills the cross axis",
          "[flex][align]") {
    flex_container_input c;
    c.width  = 200.0;
    c.height = 80.0;
    c.align_items = flex_align_items::stretch;

    std::vector<flex_item_input> items{
        item(40.0, 0.0, 0.0, 10.0),
    };

    const auto rects = flex_arrange(c, items);
    // Single line stretches to full container cross (80).
    check_rect(rects[0], {0.0, 0.0, 40.0, 80.0});
}

TEST_CASE("align_items center centers on the cross axis",
          "[flex][align]") {
    flex_container_input c;
    c.width  = 200.0;
    c.height = 80.0;
    c.align_items = flex_align_items::center;

    std::vector<flex_item_input> items{
        item(40.0, 0.0, 0.0, 20.0),
    };

    const auto rects = flex_arrange(c, items);
    // line cross size = 20 (single line, not stretched). But single line
    // with align_content stretch grows line to 80, so centering happens
    // inside an 80 tall line: y = (80 - 20)/2 = 30.
    check_rect(rects[0], {0.0, 30.0, 40.0, 20.0});
}

TEST_CASE("align_items end aligns to the far cross edge",
          "[flex][align]") {
    flex_container_input c;
    c.width  = 200.0;
    c.height = 80.0;
    c.align_items = flex_align_items::end;

    std::vector<flex_item_input> items{
        item(40.0, 0.0, 0.0, 20.0),
    };

    const auto rects = flex_arrange(c, items);
    // y = 80 - 20 = 60.
    check_rect(rects[0], {0.0, 60.0, 40.0, 20.0});
}

TEST_CASE("align_self overrides the container align_items",
          "[flex][align]") {
    flex_container_input c;
    c.width  = 200.0;
    c.height = 80.0;
    c.align_items = flex_align_items::start;

    auto a = item(40.0, 0.0, 0.0, 20.0);
    auto b = item(40.0, 0.0, 0.0, 20.0);
    b.align_self = flex_align_self::end;

    std::vector<flex_item_input> items{a, b};

    const auto rects = flex_arrange(c, items);
    // a uses container start -> y 0. b overrides to end -> y 60.
    check_rect(rects[0], {0.0,  0.0,  40.0, 20.0});
    check_rect(rects[1], {40.0, 60.0, 40.0, 20.0});
}

TEST_CASE("column direction lays items vertically",
          "[flex][direction]") {
    // Main axis is height. Two items basis 40 stack along y.
    flex_container_input c;
    c.width     = 100.0;
    c.height    = 300.0;
    c.direction = flex_direction::column;
    c.align_items = flex_align_items::start;

    std::vector<flex_item_input> items{
        item(40.0, 0.0, 0.0, 30.0),
        item(40.0, 0.0, 0.0, 30.0),
    };

    const auto rects = flex_arrange(c, items);
    // x = cross offset (start = 0), y = main offset, width = cross size,
    // height = main size.
    check_rect(rects[0], {0.0, 0.0,  30.0, 40.0});
    check_rect(rects[1], {0.0, 40.0, 30.0, 40.0});
}

TEST_CASE("column direction stretch fills width",
          "[flex][direction]") {
    flex_container_input c;
    c.width     = 100.0;
    c.height    = 300.0;
    c.direction = flex_direction::column;
    c.align_items = flex_align_items::stretch;

    std::vector<flex_item_input> items{
        item(40.0, 0.0, 0.0, 30.0),
    };

    const auto rects = flex_arrange(c, items);
    // Cross axis (width) stretches to 100.
    check_rect(rects[0], {0.0, 0.0, 100.0, 40.0});
}

TEST_CASE("row_reverse mirrors items on the main axis",
          "[flex][direction]") {
    // Container 200. Items basis 40 at positions 0 and 40 normally;
    // reversed: item0 -> 200-0-40=160, item1 -> 200-40-40=120.
    flex_container_input c;
    c.width     = 200.0;
    c.height    = 50.0;
    c.direction = flex_direction::row_reverse;
    c.align_items = flex_align_items::start;

    std::vector<flex_item_input> items{
        item(40.0, 0.0, 0.0, 10.0),
        item(40.0, 0.0, 0.0, 10.0),
    };

    const auto rects = flex_arrange(c, items);
    check_rect(rects[0], {160.0, 0.0, 40.0, 10.0});
    check_rect(rects[1], {120.0, 0.0, 40.0, 10.0});
}

TEST_CASE("order reorders items before layout",
          "[flex][order]") {
    // item0 has order 2, item1 has order 1 -> item1 first in flow.
    flex_container_input c;
    c.width  = 200.0;
    c.height = 50.0;
    c.align_items = flex_align_items::start;

    auto a = item(40.0, 0.0, 0.0, 10.0);
    a.order = 2;
    auto b = item(60.0, 0.0, 0.0, 10.0);
    b.order = 1;

    std::vector<flex_item_input> items{a, b};

    const auto rects = flex_arrange(c, items);
    // b laid first at x 0 (width 60), then a at x 60 (width 40).
    // Output is parallel to input: rects[0] is 'a', rects[1] is 'b'.
    check_rect(rects[1], {0.0,  0.0, 60.0, 10.0});
    check_rect(rects[0], {60.0, 0.0, 40.0, 10.0});
}

TEST_CASE("basis auto uses measured main size",
          "[flex][basis]") {
    // basis -1 -> uses measured_main.
    flex_container_input c;
    c.width  = 200.0;
    c.height = 50.0;
    c.align_items = flex_align_items::start;

    flex_item_input a;
    a.basis          = -1.0;
    a.measured_main  = 55.0;
    a.measured_cross = 10.0;

    flex_item_input b;
    b.basis          = 70.0;  // fixed basis ignores measured_main
    b.measured_main  = 999.0;
    b.measured_cross = 10.0;

    std::vector<flex_item_input> items{a, b};

    const auto rects = flex_arrange(c, items);
    check_rect(rects[0], {0.0,  0.0, 55.0, 10.0});
    check_rect(rects[1], {55.0, 0.0, 70.0, 10.0});
}

TEST_CASE("empty item list yields empty result",
          "[flex][edge]") {
    flex_container_input c;
    c.width  = 100.0;
    c.height = 100.0;

    const auto rects = flex_arrange(c, {});
    CHECK(rects.empty());
}

TEST_CASE("align_content start stacks lines from the top",
          "[flex][align_content]") {
    // Two lines, each cross 30, container cross 200 -> start at 0, 30.
    flex_container_input c;
    c.width  = 100.0;
    c.height = 200.0;
    c.wrap   = flex_wrap::wrap;
    c.align_items   = flex_align_items::start;
    c.align_content = flex_align_content::start;

    std::vector<flex_item_input> items{
        item(60.0, 0.0, 0.0, 30.0),
        item(60.0, 0.0, 0.0, 30.0),  // wraps to line 2
    };

    const auto rects = flex_arrange(c, items);
    check_rect(rects[0], {0.0, 0.0,  60.0, 30.0});
    check_rect(rects[1], {0.0, 30.0, 60.0, 30.0});
}

TEST_CASE("align_content center centers the line block",
          "[flex][align_content]") {
    // Two lines cross 30 each = 60. Container cross 200, free = 140,
    // offset = 70. Lines at 70 and 100.
    flex_container_input c;
    c.width  = 100.0;
    c.height = 200.0;
    c.wrap   = flex_wrap::wrap;
    c.align_items   = flex_align_items::start;
    c.align_content = flex_align_content::center;

    std::vector<flex_item_input> items{
        item(60.0, 0.0, 0.0, 30.0),
        item(60.0, 0.0, 0.0, 30.0),
    };

    const auto rects = flex_arrange(c, items);
    check_rect(rects[0], {0.0, 70.0,  60.0, 30.0});
    check_rect(rects[1], {0.0, 100.0, 60.0, 30.0});
}
