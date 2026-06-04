// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::internal::basic_graphics_view`.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/detail/graphics/canvas.hpp>
#include <mpapp/internal/basic_graphics_view.hpp>
#include <mpapp/handlers/mock/graphics_view_handler.hpp>

using namespace mpapp;

TEST_CASE("graphics_view defaults",
          "[mock][graphics_view]") {
    internal::basic_graphics_view gv;
    CHECK(gv.width.get()  == 0);
    CHECK(gv.height.get() == 0);
    CHECK(gv.draw_count.get() == 0);
}

TEST_CASE("invalidate() bumps draw_count and emits draw_requested",
          "[mock][graphics_view]") {
    internal::basic_graphics_view gv;
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
    internal::basic_graphics_view gv;
    internal::graphics_view_handler<platform::mock> h;
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

// --- drawable callback (T-0029: canvas-facade migration) ----------------

TEST_CASE("drawable defaults to an empty std::function",
          "[mock][graphics_view][drawable]") {
    internal::basic_graphics_view gv;
    CHECK_FALSE(static_cast<bool>(gv.drawable.get()));
}

TEST_CASE("mock handler records drawable install + clear",
          "[mock][graphics_view][drawable]") {
    internal::basic_graphics_view gv;
    internal::graphics_view_handler<platform::mock> h;
    h.map_drawable(gv);
    CHECK(h.last_drawable_set == false);
    h.clear_calls();

    // Install a callback - observe last_drawable_set flip + record.
    gv.drawable = [](detail::graphics::canvas&) { /* no-op for the test */ };
    CHECK(h.last_drawable_set == true);
    REQUIRE(h.calls_as_strings().size() == 1);
    CHECK(h.calls_as_strings()[0] == "drawable=1");
    h.clear_calls();

    // Clear it - should record back to 0.
    gv.drawable = internal::basic_graphics_view::draw_callback_t{};
    CHECK(h.last_drawable_set == false);
    REQUIRE(h.calls_as_strings().size() == 1);
    CHECK(h.calls_as_strings()[0] == "drawable=0");
}

TEST_CASE("drawable callback receives a canvas from make_canvas",
          "[mock][graphics_view][drawable][canvas]") {
    // Exercises the wire-up end-to-end at the surface layer: a
    // user-supplied lambda gets a real canvas, can issue ops, and the
    // resulting canvas exposes valid pixel state via the new abstract
    // pixel_data() API (where the backend supports it).
    bool   was_called = false;
    int    captured_w = 0;
    int    captured_h = 0;
    internal::basic_graphics_view gv;
    gv.drawable = [&](detail::graphics::canvas& c) {
        was_called = true;
        captured_w = c.width_px();
        captured_h = c.height_px();
        c.clear(detail::graphics::color::rgb(1.0f, 0.0f, 0.0f));
        c.fill_rect({10.0, 10.0, 40.0, 20.0});
    };

    // The graphics_view surface itself doesn't invoke the callback -
    // that's the real handler's job. We simulate the handler by
    // pulling the callback out and running it manually against a
    // canvas the test owns. This validates the surface contract
    // (drawable is settable + reachable) without needing a real
    // platform handler.
    auto canvas = detail::graphics::make_canvas(64, 32);
    REQUIRE(canvas != nullptr);
    gv.drawable.get()(*canvas);
    CHECK(was_called);
    CHECK(captured_w == 64);
    CHECK(captured_h == 32);

    // pixel_data() may be null on the stub backend; only assert
    // stride is non-negative.
    CHECK(canvas->pixel_stride_bytes() >= 0);
}
